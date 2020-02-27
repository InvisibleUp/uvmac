#ifndef DATE2SEC_H
#define DATE2SEC_H
#include "SYSDEPNS.h"

/* ************************************************************* */
/*                                                               */
/* JDATE  - Julian date calculator                               */
/*                                                               */
/* Calculates the number of days since Jan 1, 0000.              */
/*                                                               */
/* Originally written by Bradford L. Barrett (03/17/1988)        */
/* Returns an unsigned long value representing the number of     */
/* days since January 1, 0000.                                   */
/*                                                               */
/* Note: Due to the changes made by Pope Gregory XIII in the     */
/*       16th Centyry (Feb 24, 1582), dates before 1583 will     */
/*       not return a truely accurate number (will be at least   */
/*       10 days off).  Somehow, I don't think this will         */
/*       present much of a problem for most situations :)        */
/*                                                               */
/* The number returned is adjusted by 5 to facilitate day of     */
/* week calculations.  The mod of the returned value gives the   */
/* day of the week the date is.  (ie: dow = days % 7) where      */
/* dow will return 0=Sunday, 1=Monday, 2=Tuesday, etc...         */
/*                                                               */
/* ************************************************************* */
uint32_t jdate(int day, int month, int year);

uint32_t Date2MacSeconds(
	int second, int minute, int hour, int day, int month, int year
);


#endif