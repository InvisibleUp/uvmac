#include "m68k.h"
//#include "m68kcpu.h"
#include "GLOBGLUE.h"
#include <stddef.h>
#include <stdbool.h>
#include <assert.h>
#include <stdio.h>

/* --- extra global variables --- */
bool m68k_on_breakpoint = false;
bool m68k_need_singlestep = false;

/* ======================================================================== */
/* ====================== FUNCTIONS CALLED BY THE CPU ===================== */
/* ======================================================================== */

/* You will have to implement these functions */

/* read/write functions called by the CPU to access memory.
 * while values used are 32 bits, only the appropriate number
 * of bits are relevant (i.e. in write_memory_8, only the lower 8 bits
 * of value should be written to memory).
 *
 * NOTE: I have separated the immediate and PC-relative memory fetches
 *       from the other memory fetches because some systems require
 *       differentiation between PROGRAM and DATA fetches (usually
 *       for security setups such as encryption).
 *       This separation can either be achieved by setting
 *       M68K_SEPARATE_READS in m68kconf.h and defining
 *       the read functions, or by setting M68K_EMULATE_FC and
 *       making a function code callback function.
 *       Using the callback offers better emulation coverage
 *       because you can also monitor whether the CPU is in SYSTEM or
 *       USER mode, but it is also slower.
 */

/* Read from anywhere */
unsigned int  m68k_read_memory_8(unsigned int address)
{
	ATTer *r = get_address_realblock1(false, address);
	
	// check for bus error
	//assert(r != NULL);
	if (r == NULL) {
		//m68k_pulse_bus_error(); // might cause issues on Mac
		return 0;
	}
	
	address &= r->usemask;
	
	// Check if memory-mapped IO
	if (r->Access == kATTA_mmdvmask) {
		return MMDV_Access(r, 0, false, true, address);
	}
	
	// Return raw data
	uint8_t result = *(uint8_t *)(r->usebase + address);
	//fprintf(stderr, "read8  %X -> %X\n", address, result);
	return result;
}

unsigned int  m68k_read_memory_16(unsigned int address)
{
	ATTer *r = get_address_realblock1(false, address);
	
	// check for bus error
	//assert(r != NULL);
	if (r == NULL) {
		//m68k_pulse_bus_error(); // might cause issues on Mac
		return 0;
	}
	
	address &= r->usemask;
	
	// Check if memory-mapped IO
	if (r->Access == kATTA_mmdvmask) {
		return MMDV_Access(r, 0, false, false, address);
	}
	
	// Return raw data
	uint16_t result = __builtin_bswap16(*(uint16_t *)(r->usebase + address));
	//fprintf(stderr, "read16 %X -> %X\n", address, result);
	return result;
}

unsigned int  m68k_read_memory_32(unsigned int address)
{
	ATTer *r = get_address_realblock1(false, address);
	
	// check for bus error
	//assert(r != NULL);
	if (r == NULL) {
		//m68k_pulse_bus_error(); // might cause issues on Mac
		return 0;
	}
	
	address &= r->usemask;
	
	// Check if memory-mapped IO
	// TODO: check that this is the correct endianess
	if (r->Access == kATTA_mmdvmask) {
		return (MMDV_Access(r, 0, false, false, address) << 16) \
			+ MMDV_Access(r, 0, false, false, address+2);
	}
	
	// Return raw data
	uint32_t result = __builtin_bswap32(*(uint32_t *)(r->usebase + address));
	//fprintf(stderr, "read32 %X -> %X\n", address, result);
	return result;
}

#if 0
/* Read data immediately following the PC */
unsigned int  m68k_read_immediate_16(unsigned int address) {
	return m68k_read_memory_16(address);
}
unsigned int  m68k_read_immediate_32(unsigned int address) {
	return m68k_read_memory_32(address);
}

/* Read data relative to the PC */
unsigned int  m68k_read_pcrelative_8(unsigned int address) {
	return m68k_read_memory_8(address);
}
unsigned int  m68k_read_pcrelative_16(unsigned int address) {
	return m68k_read_memory_16(address);
}
unsigned int  m68k_read_pcrelative_32(unsigned int address) {
	return m68k_read_memory_32(address);
}
#endif

/* Memory access for the disassembler */
unsigned int m68k_read_disassembler_8  (unsigned int address) {
	return m68k_read_memory_8(address);
}
unsigned int m68k_read_disassembler_16 (unsigned int address) {
	return m68k_read_memory_16(address);
}
unsigned int m68k_read_disassembler_32 (unsigned int address) {
	return m68k_read_memory_32(address);
}

/* Write to anywhere */
void m68k_write_memory_8(unsigned int address, unsigned int value)
{
	ATTer *r = get_address_realblock1(true, address);
	
	// check for bus error
	assert(r != NULL);
	if (r == NULL) {
		//m68k_pulse_bus_error(); // might cause issues on Mac
		return;
	}
	
	address &= r->usemask;
	
	// Check if memory-mapped IO
	if (r->Access == kATTA_mmdvmask) {
		MMDV_Access(r, value, true, true, address);
		return;
	}
	
	// Return raw data
	*(uint8_t *)(r->usebase + address) = (uint8_t) value;
}

void m68k_write_memory_16(unsigned int address, unsigned int value)
{
	ATTer *r = get_address_realblock1(true, address);
	value = __builtin_bswap16(value);
	
	// check for bus error
	//assert (r != NULL);
	if (r == NULL) {
		//m68k_pulse_bus_error(); // might cause issues on Mac
		return;
	}
	
	address &= r->usemask;
	
	// Check if memory-mapped IO
	if (r->Access == kATTA_mmdvmask) {
		MMDV_Access(r, value, true, false, address);
		return;
	}
	
	// Return raw data
	*(uint16_t *)(r->usebase + address) = (uint16_t) value;
}

void m68k_write_memory_32(unsigned int address, unsigned int value)
{
	ATTer *r = get_address_realblock1(true, address);
	value = __builtin_bswap32(value);
	
	// check for bus error
	//assert(r != NULL);
	if (r == NULL) {
		//m68k_pulse_bus_error(); // might cause issues on Mac
		return;
	}
	
	address &= r->usemask;
	
	// Check if memory-mapped IO
	// TODO: check if proper endianess
	if (r->Access == kATTA_mmdvmask) {
		MMDV_Access(r, value & 0xFFFF0000, true, false, address);
		MMDV_Access(r, value & 0x0000FFFF, true, false, address+2);
		return;
	}
	
	// Return raw data
	*(uint32_t *)(r->usebase + address) = (uint32_t) value;
}

/* Special call to simulate undocumented 68k behavior when move.l with a
 * predecrement destination mode is executed.
 * To simulate real 68k behavior, first write the high word to
 * [address+2], and then write the low word to [address].
 *
 * Enable this functionality with M68K_SIMULATE_PD_WRITES in m68kconf.h.
 */
void m68k_write_memory_32_pd(unsigned int address, unsigned int value)
{
	m68k_write_memory_16(address+2, value & 0xFFFF0000);
	m68k_write_memory_16(address,   value & 0x0000FFFF);
}

// Initialize M68K with the proper options for the system
bool m68k_init_mac()
{
	// TODO: this, but better
	if (Use68020) {
		m68k_set_cpu_type(M68K_CPU_TYPE_68020);
	} else {
		m68k_set_cpu_type(M68K_CPU_TYPE_68000);
	}
	
	m68k_init();
	//gdbstub_init();
	return true;
}

// Thunks for old M68K emulator
uint8_t get_vm_byte(uint32_t addr) {
	return m68k_read_memory_8(addr);
}
uint16_t get_vm_word(uint32_t addr) {
	return m68k_read_memory_16(addr);
}
uint32_t get_vm_long(uint32_t addr) {
	return m68k_read_memory_32(addr);
}
void put_vm_byte(uint32_t addr, uint8_t b) {
	m68k_write_memory_8(addr, b);
}
void put_vm_word(uint32_t addr, uint16_t w) {
	m68k_write_memory_16(addr, w);
}
void put_vm_long(uint32_t addr, uint32_t l) {
	m68k_write_memory_32(addr, l);
}

void DiskInsertedPsuedoException(CPTR newpc, uint32_t data)
{
	/*uint sr;

	sr = m68ki_init_exception();
	m68ki_stack_frame_0000(REG_PPC, sr, EXCEPTION_1010);
	m68ki_jump_vector(newpc);
	
	// push data onto... something?
	m68k_set_reg(M68K_REG_A7, m68k_get_reg(NULL, M68K_REG_A7) - 4);
	m68k_write_memory_32(m68k_get_reg(NULL, M68K_REG_A7), data);*/
	return;
}

// purely for debugging purposes
void m68k_instruction_hook(uint32_t pc)
{
	if (pc == 0x4006E4) {
		fprintf(stderr, "PC: %X\n", pc);
	}
}
