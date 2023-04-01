/*
 * Copyright (c) 2012 Google Inc. All rights reserved.
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 * Modified from https://github.com/mseaborn/gdb-debug-stub
 */

#define _GNU_SOURCE
#define GDBSTUB_PORT 4014

#include <assert.h>
#ifdef _WIN32
#include <winsock2.h>
#else
#include <arpa/inet.h>
#endif
#include <signal.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "m68k.h"

struct state {
  int sock_fd;
  FILE *fp;
  int first_break;
} g_state;

// Conenct to socket when ready
void gdbstub_reconnect()
{
  if (g_state.fp != NULL) { return; }
  
  // create fd_set
  fd_set rfds;
  FD_SET(g_state.sock_fd, &rfds);
  struct timeval timeout = {0};
  
  // check if we're ready to read
  int ready = select(1, &rfds, NULL, NULL, &timeout);
  if (!ready) { return; }
  
  // make connection
  int fd = accept(g_state.sock_fd, NULL, 0);
  if (fd == 0) { return; }

  // make file descriptor
  g_state.fp = fdopen(fd, "w+");
  assert(g_state.fp != NULL);
}

// Get socket connection
static void get_connection()
{
  int sock_fd = socket(AF_INET, SOCK_STREAM, 0);
  assert(sock_fd >= 0);

  struct sockaddr_in sockaddr;
  sockaddr.sin_family = AF_INET;
  sockaddr.sin_addr.s_addr = htonl(0x7f000001); /* 127.0.0.1: localhost */
  sockaddr.sin_port = htons(GDBSTUB_PORT);

  int reuse_address = 1;
  int rc = setsockopt(sock_fd, SOL_SOCKET, SO_REUSEADDR,
                      (void *) &reuse_address, sizeof(reuse_address));
  assert(rc == 0);
  rc = bind(sock_fd, (struct sockaddr *) &sockaddr, sizeof(sockaddr));
  assert(rc == 0);

  rc = listen(sock_fd, 1);
  assert(rc == 0);
  
  g_state.sock_fd = sock_fd;
}

static int hex_to_int(char ch) {
  if ('0' <= ch && ch <= '9') {
    return ch - '0';
  } else if ('a' <= ch && ch <= 'f') {
    return ch - 'a' + 10;
  } else if ('A' <= ch && ch <= 'F') {
    return ch - 'A' + 10;
  } else {
    return 0;
  }
}

static char int_to_hex(unsigned val) {
  assert(val < 16);
  if (val < 10) {
    return val + '0';
  } else {
    return val - 10 + 'a';
  }
}

static void write_hex_byte(char *dest, uint8_t byte) {
  dest[0] = int_to_hex(byte >> 4);
  dest[1] = int_to_hex(byte & 0xf);
}

static void write_hex_bytes(char *dest, uint8_t *data, size_t size) {
  size_t index;
  for (index = 0; index < size; index++) {
    write_hex_byte(dest, data[index]);
    dest += 2;
  }
}

static int log_getc(FILE *fp) {
  int ch = getc(fp);
  if (ch == EOF) {
    // TODO: this is almost certainly a bad idea
    fprintf(stderr, "Got EOF: exiting\n");
    exit(0);
  }
  return ch;
}

/* Read a message of the format "$<data>#<checksum>". */
static void get_packet(FILE *fp, char *buffer, int buffer_size) {
  while (1) {
    /* Wait for the start character, '$', ignoring others. */
    while (1) {
      int ch = log_getc(fp);
      if (ch == '$')
        break;
      fprintf(stderr, "Unexpected char: '%c' (%i)\n", ch, ch);
    }

    int count = 0;
    uint8_t checksum = 0;
    while (1) {
      assert(count < buffer_size);
      char ch = log_getc(fp);
      if (ch == '#')
        break;
      checksum += ch;
      buffer[count++] = ch;
    }
    buffer[count] = 0;
    uint8_t received_checksum = hex_to_int(log_getc(fp)) << 4;
    received_checksum += hex_to_int(log_getc(fp));
    if (received_checksum != checksum) {
      fprintf(stderr, "got bad checksum: 0x%02x != 0x%02x\n",
              received_checksum, checksum);
      putc('-', fp);
    } else {
      putc('+', fp);
    }
    fflush(fp);
    if (received_checksum == checksum) {
      fprintf(stderr, "received: '%s'\n", buffer);
      return;
    }
  }
}

static void put_packet(FILE *fp, char *packet) {
  putc('$', fp);
  uint8_t checksum = 0;
  char *ptr;
  for (ptr = packet; *ptr != 0; ptr++) {
    assert(*ptr != '$');
    assert(*ptr != '#');
    putc(*ptr, fp);
    checksum += *ptr;
  }
  putc('#', fp);
  putc(int_to_hex(checksum >> 4), fp);
  putc(int_to_hex(checksum & 0xf), fp);
  fprintf(stderr, "sent: '%s'\n", packet);
  /* Look for acknowledgement character. */
  int ch = log_getc(fp);
  if (ch != '+') {
    fprintf(stderr, "Unexpected ack char: '%c' (%i)\n", ch, ch);
  }
}
struct gdb_regs {
  uint32_t D0;		/* Data registers */
  uint32_t D1;
  uint32_t D2;
  uint32_t D3;
  uint32_t D4;
  uint32_t D5;
  uint32_t D6;
  uint32_t D7;
  uint32_t A0;		/* Address registers */
  uint32_t A1;
  uint32_t A2;
  uint32_t A3;
  uint32_t A4;
  uint32_t A5;
  uint32_t A6;
  uint32_t A7;
  uint32_t PC;		/* Program Counter */
  uint32_t SR;		/* Status Register */
  uint32_t SP;		/* The current Stack Pointer (located in A7) */
  uint32_t USP;		/* User Stack Pointer */
  uint32_t ISP;		/* Interrupt Stack Pointer */
  uint32_t MSP;		/* Master Stack Pointer */
  uint32_t SFC;		/* Source Function Code */
  uint32_t DFC;		/* Destination Function Code */
  uint32_t VBR;		/* Vector Base Register */
  uint32_t CACR;		/* Cache Control Register */
  uint32_t CAAR;		/* Cache Address Register */
};

static void copy_regs_to_gdb(struct gdb_regs *regs)
{
  memset(regs, 0, sizeof(*regs));
  regs->D0   = m68k_get_reg(NULL, M68K_REG_D0);
  regs->D1   = m68k_get_reg(NULL, M68K_REG_D1);
  regs->D2   = m68k_get_reg(NULL, M68K_REG_D2);
  regs->D3   = m68k_get_reg(NULL, M68K_REG_D3);
  regs->D4   = m68k_get_reg(NULL, M68K_REG_D4);
  regs->D5   = m68k_get_reg(NULL, M68K_REG_D5);
  regs->D6   = m68k_get_reg(NULL, M68K_REG_D6);
  regs->D7   = m68k_get_reg(NULL, M68K_REG_D7);
  regs->A0   = m68k_get_reg(NULL, M68K_REG_A0);
  regs->A1   = m68k_get_reg(NULL, M68K_REG_A1);
  regs->A2   = m68k_get_reg(NULL, M68K_REG_A2);
  regs->A3   = m68k_get_reg(NULL, M68K_REG_A3);
  regs->A4   = m68k_get_reg(NULL, M68K_REG_A4);
  regs->A5   = m68k_get_reg(NULL, M68K_REG_A5);
  regs->A6   = m68k_get_reg(NULL, M68K_REG_A6);
  regs->A7   = m68k_get_reg(NULL, M68K_REG_A7);
  regs->PC   = m68k_get_reg(NULL, M68K_REG_PC);
  regs->SR   = m68k_get_reg(NULL, M68K_REG_SR);
  regs->SP   = m68k_get_reg(NULL, M68K_REG_SP);
  regs->USP  = m68k_get_reg(NULL, M68K_REG_USP);
  regs->ISP  = m68k_get_reg(NULL, M68K_REG_ISP);
  regs->MSP  = m68k_get_reg(NULL, M68K_REG_MSP);
  regs->SFC  = m68k_get_reg(NULL, M68K_REG_SFC);
  regs->DFC  = m68k_get_reg(NULL, M68K_REG_DFC);
  regs->VBR  = m68k_get_reg(NULL, M68K_REG_VBR);
  regs->CACR = m68k_get_reg(NULL, M68K_REG_CACR);
  regs->CAAR = m68k_get_reg(NULL, M68K_REG_CAAR);
}

static void signal_handler(int signum, siginfo_t *info, void *context) {
  if (!g_state.first_break) {
    char msg[100];
    snprintf(msg, sizeof(msg), "S%02x", signum);
    put_packet(g_state.fp, msg);
  }
  g_state.first_break = 0;

  /* Unset the trap flag in case we were single-stepping before.
     TODO: This should not be unconditional. */
  m68k_need_singlestep = false;
  m68k_on_breakpoint = true;

  while (1) {
    char request[100];
    get_packet(g_state.fp, request, sizeof(request));

    switch (request[0]) {
    case '?': /* query stopped status */
      {
        char reply[10];
        snprintf(reply, sizeof(reply), "S%02x", signum);
        put_packet(g_state.fp, reply);
      }
      break;
    case 'g': /* read registers */
      {
        char reply[1000];
        struct gdb_regs regs;
        copy_regs_to_gdb(&regs);
        write_hex_bytes(reply, (uint8_t *) &regs, sizeof(regs));
        reply[sizeof(regs) * 2] = 0;
        put_packet(g_state.fp, reply);
        break;
      }
    case 'm': /* read memory */
      {
        char *rest;
        uintptr_t mem_addr = strtoll(request + 1, &rest, 16);
        assert(*rest == ',');
        size_t mem_size = strtoll(rest + 1, &rest, 16);
        assert(*rest == 0);
        char reply[1000];
        write_hex_bytes(reply, (uint8_t *) mem_addr, mem_size);
        reply[mem_size * 2] = 0;
        put_packet(g_state.fp, reply);
        break;
      }
    case 'c': /* continue */
      m68k_on_breakpoint = false;
      return;
    case 's': /* single step */
      m68k_need_singlestep = true;
      return;
    default:
      put_packet(g_state.fp, "");
      break;
    }
  }
}

void gdbstub_init() {
  get_connection();

  struct sigaction act;
  memset(&act, 0, sizeof(act));
  act.sa_sigaction = signal_handler;
  sigemptyset(&act.sa_mask);
  act.sa_flags = SA_SIGINFO;
  int rc = sigaction(SIGTRAP, &act, NULL);
  assert(rc == 0);

  g_state.first_break = 1;
  g_state.fp = NULL;
}

#undef _GNU_SOURCE
