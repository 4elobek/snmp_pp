/*_############################################################################
  _## 
  _##  msec.cpp  
  _##
  _##  SNMP++ v3.3
  _##  -----------------------------------------------
  _##  Copyright (c) 2001-2013 Jochen Katz, Frank Fock
  _##
  _##  This software is based on SNMP++2.6 from Hewlett Packard:
  _##  
  _##    Copyright (c) 1996
  _##    Hewlett-Packard Company
  _##  
  _##  ATTENTION: USE OF THIS SOFTWARE IS SUBJECT TO THE FOLLOWING TERMS.
  _##  Permission to use, copy, modify, distribute and/or sell this software 
  _##  and/or its documentation is hereby granted without fee. User agrees 
  _##  to display the above copyright notice and this license notice in all 
  _##  copies of the software and any documentation of the software. User 
  _##  agrees to assume all liability for the use of the software; 
  _##  Hewlett-Packard and Jochen Katz make no representations about the 
  _##  suitability of this software for any purpose. It is provided 
  _##  "AS-IS" without warranty of any kind, either express or implied. User 
  _##  hereby grants a royalty-free license to any and all derivatives based
  _##  upon this software code base. 
  _##  
  _##########################################################################*/
/*
  Copyright (c) 1999
  Hewlett-Packard Company

  ATTENTION: USE OF THIS SOFTWARE IS SUBJECT TO THE FOLLOWING TERMS.
  Permission to use, copy, modify, distribute and/or sell this software
  and/or its documentation is hereby granted without fee. User agrees
  to display the above copyright notice and this license notice in all
  copies of the software and any documentation of the software. User
  agrees to assume all liability for the use of the software; Hewlett-Packard
  makes no representations about the suitability of this software for any
  purpose. It is provided "AS-IS" without warranty of any kind,either express
  or implied. User hereby grants a royalty-free license to any and all
  derivatives based upon this software code base.
*/
char msec_cpp_version[]="@(#) SNMP++ $Id$";

#include <libsnmp.h>

#include "snmp_pp/msec.h"
#include "snmp_pp/smival.h"
#include "snmp_pp/config_snmp_pp.h"


namespace Snmp_pp {

int operator==(const msec &t1, const msec &t2)
{
  return((t1.m_time.tv_sec == t2.m_time.tv_sec) &&
	 (t1.m_time.tv_usec == t2.m_time.tv_usec));
}

int operator!=(const msec &t1, const msec &t2)
{
  return((t1.m_time.tv_sec != t2.m_time.tv_sec) ||
	 (t1.m_time.tv_usec != t2.m_time.tv_usec));
}

int operator<(const msec &t1, const msec &t2)
{
  if (t1.IsInfinite()) return 0;
  if (t2.IsInfinite()) return 1;
  if ((t1.m_time.tv_sec < t2.m_time.tv_sec) ||
      ((t1.m_time.tv_sec == t2.m_time.tv_sec) &&
       (t1.m_time.tv_usec < t2.m_time.tv_usec)))
    return 1;
  return 0;
}

int operator>(const msec &t1, const msec &t2)
{
  if (t2.IsInfinite()) return 0;
  if (t1.IsInfinite()) return 1;
  if ((t1.m_time.tv_sec > t2.m_time.tv_sec) ||
      ((t1.m_time.tv_sec == t2.m_time.tv_sec) &&
       (t1.m_time.tv_usec > t2.m_time.tv_usec)))
    return 1;
  return 0;
}

msec &msec::operator-=(const long millisec)
{
  timeval t1;

  // create a timeval
  t1.tv_sec = millisec / 1000;
  t1.tv_usec = (millisec % 1000) * 1000;
  // subtract it from this
  *this -= t1; // add m_changed = true if this line is removed!
  return *this;
}

msec &msec::operator-=(const timeval &t1)
{
  long tmp_usec = t1.tv_usec/1000;// convert usec to millisec
  if (!this->IsInfinite())
  {
    if (m_time.tv_usec < tmp_usec) {
      // borrow
      m_time.tv_sec--;
      m_time.tv_usec += 1000;
    }
    m_time.tv_usec -= tmp_usec;
    m_time.tv_sec -= t1.tv_sec;
  }
  m_changed = true;
  return *this;
}

msec &msec::operator+=(const long millisec)
{
  timeval t1;

  // create a timeval
  t1.tv_sec = millisec / 1000;
  t1.tv_usec = (millisec % 1000) * 1000;
  // add it to this
  *this += t1; // add m_changed = true if this line is removed!
  return *this;
}

msec &msec::operator+=(const timeval &t1)
{
  long tmp_usec = t1.tv_usec/1000;// convert usec to millisec
  if (!this->IsInfinite())
  {
    m_time.tv_usec += tmp_usec;
    if (m_time.tv_usec >= 1000) {
      // carry
      m_time.tv_sec +=  m_time.tv_usec / 1000;
      m_time.tv_usec = m_time.tv_usec % 1000;
    }
    m_time.tv_sec += t1.tv_sec;
  }
  m_changed = true;
  return *this;
}

msec &msec::operator=(const timeval &t1)
{
  m_time.tv_sec  = t1.tv_sec;
  m_time.tv_usec = t1.tv_usec/1000; // convert usec to millisec
  m_changed = true;
  return *this;
}

void msec::refresh()
{
  struct timezone tzone;
  gettimeofday((timeval *)&m_time, &tzone);
  m_time.tv_usec /= 1000; // convert usec to millisec
  m_changed = true;
}

#ifndef MAX_ALARM
#define MAX_ALARM 1000000000L
#endif

void msec::GetDelta(const msec &future, timeval &timeout) const
{
  if (future.IsInfinite())
  {
    timeout.tv_sec = MAX_ALARM; // max allowable select timeout
    timeout.tv_usec = 0;
  }
  else if (future > *this)
  {
    if (future.m_time.tv_usec < m_time.tv_usec)
    {
      timeout.tv_sec  = future.m_time.tv_sec  - 1    - m_time.tv_sec;
      timeout.tv_usec = future.m_time.tv_usec + 1000 - m_time.tv_usec;
    }
    else
    {
      timeout.tv_sec  = future.m_time.tv_sec  - m_time.tv_sec;
      timeout.tv_usec = future.m_time.tv_usec - m_time.tv_usec;
    }
    timeout.tv_usec *= 1000 ;// convert back to usec
  }
  else // Never give back negative timeval's they make select() hurl
  {
    timeout.tv_sec = 0;
    timeout.tv_usec = 0;
  }
}

// FIXME: does not print years and days!
const char *msec::get_printable() const
{
  if (m_changed == false) return m_output_buffer;

  char msec_buffer[5];
  msec *nc_this = PP_CONST_CAST(msec*, this);

  struct tm stm;
  localtime_r((const time_t *)&m_time.tv_sec, &stm);
  strftime(nc_this->m_output_buffer, sizeof(m_output_buffer),
           "%H:%M:%S.", &stm);

  sprintf(msec_buffer, "%.3ld", (long)m_time.tv_usec);
  strcat(nc_this->m_output_buffer, msec_buffer);

  nc_this->m_changed = false;

  return m_output_buffer;
}

} // end of namespace Snmp_pp
