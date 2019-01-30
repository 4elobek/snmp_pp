/*_############################################################################
  _##
  _##  address.cpp
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
/*===================================================================

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

  A D D R E S S. C P P

  ADDRESS CLASS IMPLEMENTATION

  DESIGN + AUTHOR:  Peter E. Mellquist

  DESCRIPTION:      Implementation file for Address classes.
=====================================================================*/
char address_cpp_version[]="@(#) SNMP++ $Id$";

#include <libsnmp.h>

#include "snmp_pp/address.h"
#include "snmp_pp/v3.h"

namespace Snmp_pp {

/* Borlands isdigit has a bug */
#ifdef __BCPLUSPLUS__
#define my_isdigit(c) ((c) >= '0' && (c) <= '9')
#else
#define my_isdigit isdigit
#endif

#ifdef ADDRESS_DEBUG
#define ADDRESS_TRACE debugprintf(0, "ADDRESS %p Enter %s", this, __PRETTY_FUNCTION__)
#define ADDRESS_TRACE2 debugprintf(0, "ADDRESS op Enter %s", __PRETTY_FUNCTION__)
#else
#define ADDRESS_TRACE
#define ADDRESS_TRACE2
#endif

//=================================================================
//======== Abstract Address Class Implementation ==================
//=================================================================

Address::Address()
  : addr_changed(true), valid_flag(false)
{
  ADDRESS_TRACE;

  memset(address_buffer, 0, sizeof(unsigned char)*ADDRBUF);
}

//------------[ Address::trim_white_space( char * ptr) ]------------
// destructive trim white space
void Address::trim_white_space(char *ptr)
{
  ADDRESS_TRACE;

  char *tmp = ptr;                               // init
  while (*tmp==' ') tmp++;                       // skip leading white space
  while (*tmp && (*tmp != ' ')) *ptr++ = *tmp++; // move string to beginning
  *ptr = 0;                                      // set end of string
}

// Reset the object
void Address::clear()
{
  addr_changed = true;
  valid_flag = false;
  memset(address_buffer, 0, sizeof(unsigned char)*ADDRBUF);
}

//-----------------------------------------------------------------------
// overloaded equivalence operator, are two addresses equal?
int operator==(const Address &lhs, const Address &rhs)
{
  ADDRESS_TRACE2;

  return (strcmp((const char*)lhs, (const char*)rhs) == 0);
}

//------------------------------------------------------------------
// overloaded > operator, is a1 > a2
int operator>(const Address &lhs, const Address &rhs)
{
  ADDRESS_TRACE2;

  return (strcmp((const char*)lhs, (const char*)rhs) > 0);
}

//-----------------------------------------------------------------
// overloaded < operator, is a1 < a2
int operator<(const Address &lhs, const Address &rhs)
{
  ADDRESS_TRACE2;

  return (strcmp((const char*)lhs, (const char*)rhs) < 0);
}

//------------------------------------------------------------------
// equivlence operator overloaded, are an address and a string equal?
int operator==(const Address &lhs, const char *rhs)
{
  ADDRESS_TRACE2;

  if (!rhs && !lhs.valid())
    return true;
  else if (!rhs) {
    return false;
  }
  if (strcmp((const char *)lhs, rhs) == 0)
    return true;
  return false;
}

//------------------------------------------------------------------
// overloaded > , is a > inaddr
int operator>(const Address &lhs, const char *rhs)
{
  ADDRESS_TRACE2;

  if (!rhs)
    return lhs.valid();  // if lhs valid then > NULL, else invalid !> NULL
  if (strcmp((const char *)lhs, rhs) > 0)
    return true;
  return false;
}

//------------------------------------------------------------------
// overloaded >= , is a >= inaddr
int operator>=(const Address &lhs, const char *rhs)
{
  ADDRESS_TRACE2;

  if (!rhs)
    return true; // always >= NULL
  if (strcmp((const char *)lhs, rhs) >= 0)
    return true;
  return false;
}

//-----------------------------------------------------------------
// overloaded < , are an address and a string equal?
int operator<(const Address &lhs, const char *rhs)
{
  ADDRESS_TRACE2;

  if (!rhs)
    return false; // always >= NULL
  if (strcmp((const char *)lhs, rhs) < 0)
    return true;
  return false;
}

//-----------------------------------------------------------------
// overloaded <= , is a <= inaddr
int operator<=(const Address &lhs, const char *rhs)
{
  ADDRESS_TRACE2;

  if (!rhs)
    return !lhs.valid(); // invalid == NULL, else valid > NULL
  if (strcmp((const char *)lhs, rhs) <= 0)
    return true;
  return false;
}

//=====================================================================
//============ IPAddress Implementation ===============================
//=====================================================================

//-------[ construct an IP address with no agrs ]----------------------
IpAddress::IpAddress()
  : Address(), iv_friendly_name_status(0), ip_version(version_ipv4), have_ipv6_scope(false)
{
  ADDRESS_TRACE;

  // always initialize what type this object is
  smival.syntax = sNMP_SYNTAX_IPADDR;
  smival.value.string.len = IPLEN;
  smival.value.string.ptr = address_buffer;
}

//-------[ construct an IP address with a string ]---------------------
IpAddress::IpAddress(const char *inaddr)
  : Address()
{
  ADDRESS_TRACE;

  // always initialize what type this object is
  smival.syntax = sNMP_SYNTAX_IPADDR;
  smival.value.string.len = IPLEN;
  smival.value.string.ptr = address_buffer;

  // parse_address initializes valid, address_buffer & iv_friendly_name
  valid_flag = parse_address(inaddr);
}

//-----[ IP Address copy constructor ]---------------------------------
IpAddress::IpAddress(const IpAddress &ipaddr)
    : iv_friendly_name_status(0), ip_version(ipaddr.ip_version),
      have_ipv6_scope(ipaddr.have_ipv6_scope)
{
  ADDRESS_TRACE;

  // always initialize what type this object is
  smival.syntax = sNMP_SYNTAX_IPADDR;
  smival.value.string.len = ipaddr.smival.value.string.len;
  smival.value.string.ptr = address_buffer;

  valid_flag = ipaddr.valid_flag;
  if (valid_flag)
  {
    // copy the address data
    memcpy(address_buffer, ipaddr.address_buffer, smival.value.string.len);
    // and the friendly name
    iv_friendly_name = ipaddr.iv_friendly_name;

    if (!ipaddr.addr_changed)
    {
      memcpy(output_buffer, ipaddr.output_buffer,
             sizeof(unsigned char) * OUTBUFF);
      addr_changed = false;
    }
  }
}

//-----[ construct an IP address with a GenAddress ]---------------------
IpAddress::IpAddress(const GenAddress &genaddr)
  : iv_friendly_name_status(0)
{
  ADDRESS_TRACE;

  // always initialize what type this object is
  smival.syntax = sNMP_SYNTAX_IPADDR;
  smival.value.string.len = IPLEN;
  smival.value.string.ptr = address_buffer;

  output_buffer[0]=0;

  // allow use of an ip or udp genaddress
  valid_flag = genaddr.valid();
  if (valid_flag)
  {
    if (genaddr.get_type() == type_ip)
    {
      // copy in the IP address data
      *this = genaddr.cast_ipaddress();
      return;
    }
    else if (genaddr.get_type() == type_udp)
    {
      // copy in the IP address data
      *this = genaddr.cast_udpaddress();
      return;
    }
  }
  valid_flag = false;
  addr_changed = true;
}

//-----[ IP Address general = operator ]-------------------------------
SnmpSyntax& IpAddress::operator=(const SnmpSyntax &val)
{
  ADDRESS_TRACE;

  if (this == &val) return *this; // protect against assignment from itself

  addr_changed = true;
  valid_flag = false;        // will get set TRUE if really valid
  iv_friendly_name.clear();

  if (val.valid())
  {
    switch (val.get_syntax())
    {
      case sNMP_SYNTAX_IPADDR:
      case sNMP_SYNTAX_OCTETS:
        if ((((IpAddress &)val).smival.value.string.len == IPLEN) ||
            (((IpAddress &)val).smival.value.string.len == UDPIPLEN))
        {
          memcpy(address_buffer,
                 ((IpAddress &)val).smival.value.string.ptr, IPLEN);
          valid_flag = true;
          ip_version = version_ipv4;
          smival.value.string.len = IPLEN;
        }
        else if ((((IpAddress &)val).smival.value.string.len == IP6LEN_NO_SCOPE) ||
                 (((IpAddress &)val).smival.value.string.len == UDPIP6LEN_NO_SCOPE))
        {
          memcpy(address_buffer,
                 ((IpAddress &)val).smival.value.string.ptr, IP6LEN_NO_SCOPE);
          valid_flag = true;
          ip_version = version_ipv6;
          smival.value.string.len = IP6LEN_NO_SCOPE;
          have_ipv6_scope = false;
        }
        else if ((((IpAddress &)val).smival.value.string.len == IP6LEN_WITH_SCOPE) ||
                 (((IpAddress &)val).smival.value.string.len == UDPIP6LEN_WITH_SCOPE))
        {
          memcpy(address_buffer,
                 ((IpAddress &)val).smival.value.string.ptr, IP6LEN_WITH_SCOPE);
          valid_flag = true;
          ip_version = version_ipv6;
          smival.value.string.len = IP6LEN_WITH_SCOPE;
          have_ipv6_scope = true;
        }
        break;

        // NOTE: as a value add, other types could have "logical"
        // mappings, i.e. integer32 and unsigned32
    }
  }
  return *this;
}

Address& IpAddress::operator=(const Address &val)
{
  ADDRESS_TRACE;

  if (this == &val) return *this; // protect against assignment from itself

  addr_changed = true;
  valid_flag = false;        // will get set TRUE if really valid
  iv_friendly_name.clear();

  if (val.valid())
  {
    switch (val.get_syntax())
    {
      case sNMP_SYNTAX_IPADDR:
      case sNMP_SYNTAX_OCTETS:
        if ((((IpAddress &)val).smival.value.string.len == IPLEN) ||
            (((IpAddress &)val).smival.value.string.len == UDPIPLEN))
        {
          MEMCPY(address_buffer,
                 ((IpAddress &)val).smival.value.string.ptr, IPLEN);
          valid_flag = true;
          ip_version = version_ipv4;
          smival.value.string.len = IPLEN;
        }
        else if ((((IpAddress &)val).smival.value.string.len == IP6LEN_NO_SCOPE) ||
                 (((IpAddress &)val).smival.value.string.len == UDPIP6LEN_NO_SCOPE))
        {
          MEMCPY(address_buffer,
                 ((IpAddress &)val).smival.value.string.ptr, IP6LEN_NO_SCOPE);
          valid_flag = true;
          ip_version = version_ipv6;
          smival.value.string.len = IP6LEN_NO_SCOPE;
          have_ipv6_scope = false;
        }
        else if ((((IpAddress &)val).smival.value.string.len == IP6LEN_WITH_SCOPE) ||
                 (((IpAddress &)val).smival.value.string.len == UDPIP6LEN_WITH_SCOPE))
        {
          MEMCPY(address_buffer,
                 ((IpAddress &)val).smival.value.string.ptr, IP6LEN_WITH_SCOPE);
          valid_flag = true;
          ip_version = version_ipv6;
          smival.value.string.len = IP6LEN_WITH_SCOPE;
          have_ipv6_scope = true;
        }
        break;

        // NOTE: as a value add, other types could have "logical"
        // mappings, i.e. integer32 and unsigned32
    }
  }
  return *this;
}
//------[ assignment to another ipaddress object overloaded ]-----------------
IpAddress& IpAddress::operator=(const IpAddress &ipaddr)
{
  ADDRESS_TRACE;

  if (this == &ipaddr) return *this; // protect against assignment from itself

  valid_flag = ipaddr.valid_flag;
  iv_friendly_name.clear();

  if (valid_flag)
  {
    if (ipaddr.ip_version == version_ipv4)
    {
      memcpy(address_buffer, ipaddr.address_buffer, IPLEN);
      ip_version = version_ipv4;
      smival.value.string.len = IPLEN;
    }
    else
    {
      if (ipaddr.have_ipv6_scope)
      {
        memcpy(address_buffer, ipaddr.address_buffer, IP6LEN_WITH_SCOPE);
        ip_version = version_ipv6;
        smival.value.string.len = IP6LEN_WITH_SCOPE;
        have_ipv6_scope = true;
      }
      else
      {
        memcpy(address_buffer, ipaddr.address_buffer, IP6LEN_NO_SCOPE);
        ip_version = version_ipv6;
        smival.value.string.len = IP6LEN_NO_SCOPE;
        have_ipv6_scope = false;
      }
    }
    iv_friendly_name = ipaddr.iv_friendly_name;

    if (ipaddr.addr_changed)
      addr_changed = true;
    else
    {
      memcpy(output_buffer, ipaddr.output_buffer,
             sizeof(unsigned char) * OUTBUFF);
      addr_changed = false;
    }
  }
  else
    addr_changed = true;
  return *this;
}

//-------[ return the friendly name ]----------------------------------
const char *IpAddress::friendly_name(int &status)
{
  ADDRESS_TRACE;

  if ((iv_friendly_name.length() == 0) && (valid_flag))
    this->addr_to_friendly();
  status = iv_friendly_name_status;
  return iv_friendly_name.c_str();
}

// Clone as OctetStr (binary string)
OctetStr *IpAddress::clone_as_hex() const
{
 ADDRESS_TRACE;

 OctetStr *hex = new OctetStr();
 hex->set_len(get_length());
 for (int i=0; i < get_length(); i++)
 {
   (*hex)[i] = address_buffer[i];
 }
 return hex;
}

// parse a dotted string
int IpAddress::parse_dotted_ipstring(const char *inaddr)
{
  ADDRESS_TRACE;

  int token_count=0;
  char temp[30];  // temp buffer for destruction

  // check len, an ip can never be bigger than 15
  // 123456789012345
  // XXX.XXX.XXX.XXX
  if (!inaddr || (strlen(inaddr) >= sizeof(temp))) return false;

  strcpy(temp, inaddr);
  trim_white_space(temp);
  if (strlen(temp) > 15) return false;

  /* Check for the following:
   * - exactly three dots
   * - no dot at begin or end
   * - at least a digit between two dots
   * - only dots and digits allowed
   */
  char *ptr = temp;
  int dot_count = 0;
  bool last_char_was_dot = true;

  while (*ptr)
  {
    if (*ptr == '.')
    {
      if (last_char_was_dot) return false;
      ++dot_count;
      last_char_was_dot = true;
    }
    else if (my_isdigit(*ptr))
    {
      last_char_was_dot = false;
    }
    else
      return false;
    ++ptr;
  }
  if ((dot_count != 3) || (last_char_was_dot))
    return false;

  ptr = temp;
  while (*ptr)
  {
    unsigned long number = 0;

    if (*ptr == '.') ++ptr;    // skip over the dot

    // grab a digit token and convert it to a long int
    int digits = 0;
    while ((*ptr) && (*ptr != '.'))
    {
      number = (number * 10) + *(ptr++) - '0';
      ++digits;
    }
    if (digits > 3) return false;
    if (number > 255) return false;

    // stuff the value into the array and bump the counter
    address_buffer[token_count++]= (unsigned char) number;
  }

  ip_version = version_ipv4;
  smival.value.string.len = IPLEN;
  return true;
}

#define ATOI(x)    if      ((x >= 48) && (x <= 57)) x = x-48; /* 0-9 */ \
                   else if ((x >= 97) && (x <=102)) x = x-87; /* a-f */ \
                   else if ((x >= 65) && (x <= 70)) x = x-55; /* A-F */ \
                   else x=0

// parse a coloned string
int IpAddress::parse_coloned_ipstring(const char *inaddr)
{
  ADDRESS_TRACE;

  unsigned char tmp_address_buffer[ADDRBUF];
  char temp[60];  // temp buffer for destruction

  // check len, an ipv6 can never be bigger than 39 + 11
  // 123456789012345678901234567890123456789
  // 1BCD:2BCD:3BCD:4BCD:5BCD:6BCD:7BCD:8BCD%4123456789
  if (!inaddr || (strlen(inaddr) >= sizeof(temp))) return false;
  strcpy(temp, inaddr);
  trim_white_space(temp);

  // first check for ipv6 scope
  unsigned int scope = 0;
  bool have_scope = false;

  {
      int scope_pos;
      for (int i=strlen(temp)-1; i >=0 ; i--)
      {
          if (temp[i] == '%')
          {
              scope_pos = i;
              have_scope = true;
              break;
          }
          if (!isdigit(temp[i]))
              break;
      }
      if (have_scope)
      {
          temp[scope_pos] = 0;
          scope = atol(temp + scope_pos + 1);
      }
  }

  if (strlen(temp) > 39) return false;

  char *in_ptr = temp;
  char *out_ptr = (char*)tmp_address_buffer;
  char *end_first_part = NULL;
  char second[39];
  int second_used = false;
  int colon_count = 0;
  int had_double_colon = false;
  int last_was_colon = false;
  int had_dot = false;
  int dot_count = 0;
  int digit_count = 0;
  char digits[4];
  char last_deliminiter = 0;

  while (*in_ptr != 0)
  {
    if (*in_ptr == '.')
    {
      last_deliminiter = *in_ptr;
      had_dot = true;
      dot_count++;
      if (dot_count > 3)
        return false;
      if ((digit_count > 3) || (digit_count < 1))
        return false;
      for (int i=0; i<digit_count; i++)
        if (!my_isdigit(digits[i]))
          return false;
      digits[digit_count] = 0;
      int value = atoi(digits);
      if ((value > 0) && (value <= 255))
        *out_ptr++ = (unsigned char) value;
      else
      {
        if (strcmp(digits, "0") == 0)
          *out_ptr++ = (unsigned char) 0;
        else
          return false;
      }
      digit_count = 0;
    }
    else if (*in_ptr == ':')
    {
      last_deliminiter = *in_ptr;

      if (had_dot)
        return false; // don't allow : after a dot

      if (digit_count)
      {
        // move digits to right
        {
          for (int i=0; i<digit_count; i++)
          {
            ATOI(digits[digit_count - 1 - i]);
            digits[3-i] = digits[digit_count - 1 - i];
          }
        }
        {
          for (int i=0; i<4-digit_count; i++)
          digits[i] = 0;
        }
        {
          // pack two digits into one byte
          for (int i=0; i < 4; i += 2)
          {
            unsigned char c = digits[i];
            unsigned char d = digits[i+1];
            *out_ptr++ = (c*16 + d);
          }
        }
        digit_count = 0;
      }
      colon_count++;
      if (last_was_colon)
      {
        if (had_double_colon)
          return false;
        end_first_part = out_ptr;
        out_ptr = second;
        second_used = true;
        had_double_colon = true;
      }
      else
      {
        last_was_colon = true;
      }
    }
    else
    {
      if (digit_count >= 4)
        return false;
      if (!isxdigit(*in_ptr))
        return false;
      digits[digit_count] = tolower(*in_ptr);

      digit_count++;
      if (digit_count > 4)
        return false;
      last_was_colon = 0;
    }
    in_ptr++;
  }

  // put last bytes from digits into buffer
  if (digit_count)
  {
    if (last_deliminiter == ':')
    {
      {
        // move digits to right
        for (int i=0; i<digit_count; i++)
        {
          ATOI(digits[digit_count - 1 - i]);
          digits[3-i] = digits[digit_count - 1 - i];
        }
      }
      {
        for (int i=0; i<4-digit_count; i++)
          digits[i] = 0;
      }
      {
        // pack two digits into one byte
        for (int i=0; i < 4; i += 2)
        {
          unsigned char c = digits[i];
          unsigned char d = digits[i+1];
          *out_ptr++ = (c*16 + d);
        }
      }
      digit_count = 0;
    }
    else if (last_deliminiter == '.')
    {
      if ((digit_count > 3) || (digit_count < 1))
        return false;
      for (int i=0; i<digit_count; i++)
        if (!my_isdigit(digits[i]))
          return false;
      digits[digit_count] = 0;
      int value = atoi(digits);
      if ((value > 0) && (value <= 255))
        *out_ptr++ = (unsigned char) value;
      else
      {
        if (strcmp(digits, "0") == 0)
          *out_ptr++ = (unsigned char) 0;
        else
          return false;
      }
      //digit_count = 0;
    }
    else
      return false;
  }

  // must have between two and seven colons
  if ((colon_count > 7) || (colon_count < 2))
    return false;

  // if there was a dot there must be three of them
  if ((dot_count > 0) && (dot_count != 3))
    return false;

  if (second_used)
  {
    int len_first  = SAFE_INT_CAST(end_first_part - (char*)tmp_address_buffer);
    int len_second = SAFE_INT_CAST(out_ptr - second);

    int i;
    for (i=0; i<IP6LEN_NO_SCOPE-(len_first + len_second); i++)
      *end_first_part++ = 0;
    for (i=0; i<len_second; i++)
      *end_first_part++ = second[i];
  }

  if (!end_first_part)
    end_first_part = out_ptr;

  // check for short address
  if (end_first_part - (char*)tmp_address_buffer != IP6LEN_NO_SCOPE)
    return false;

  ip_version = version_ipv6;
  if (have_scope)
      smival.value.string.len = IP6LEN_WITH_SCOPE;
  else
      smival.value.string.len = IP6LEN_NO_SCOPE;

  memcpy(address_buffer, tmp_address_buffer, ADDRBUF);

  if (have_scope)
  {
    unsigned int *scope_p = (unsigned int*)(address_buffer + IP6LEN_NO_SCOPE);
    *scope_p = htonl(scope);
    have_ipv6_scope = true;
  }
  else
      have_ipv6_scope = false;

  return true;
}

#undef ATOI


//-----[ IP Address parse Address ]---------------------------------
bool IpAddress::parse_address(const char *inaddr)
{
  ADDRESS_TRACE;

  addr_changed = true;

  // initialize the friendly_name member variable
  iv_friendly_name.clear();
  iv_friendly_name_status = 0;

  // is this a dotted IP notation string or a friendly name
  if (parse_dotted_ipstring(inaddr))
    return true; // since this is a valid dotted string don't do any DNS
  else if (parse_coloned_ipstring(inaddr))
    return true; // since this is a valid ipv6 string don't do any DNS

  struct addrinfo hints, *res = 0;
  int error;
  // XXX ensure that MAX_FRIENDLY_NAME keeps greater than INET6_ADDRSTRLEN
  char ds[MAX_FRIENDLY_NAME];

  memset(&hints, 0, sizeof(hints));
  hints.ai_flags = AI_CANONNAME;
#ifdef AI_ADDRCONFIG
  hints.ai_flags |= AI_ADDRCONFIG;
#endif
  error = getaddrinfo(inaddr, 0, &hints, &res);
  if (error)
  {
    /* errx(1, "%s", gai_strerror(error)); */
    iv_friendly_name_status = error;
    return false;
  }
  else
  {
    if (res->ai_family == AF_INET6)
    {
      if (!inet_ntop(AF_INET6,
          &((struct sockaddr_in6 *)(res->ai_addr))->sin6_addr,
          ds, sizeof(ds)-1))
      {
        freeaddrinfo(res);
        return false;
      }
    }
    else
    if (res->ai_family == AF_INET)
    {
      // now lets check out the coloned string
      if (!inet_ntop(AF_INET,
          &((struct sockaddr_in *)(res->ai_addr))->sin_addr,
          ds, sizeof(ds)-1))
      {
        freeaddrinfo(res);
        return false;
      }
    }

    debugprintf(4, "from inet_ntop: %s", ds);
    if (
       (res->ai_family == AF_INET6 && !parse_coloned_ipstring(ds)) ||
       (res->ai_family == AF_INET && !parse_dotted_ipstring(ds))
    )
    {
      freeaddrinfo(res);
      return false;
    }

    freeaddrinfo(res);
    iv_friendly_name_status = 0;
    // save the friendly name
    iv_friendly_name = inaddr;

    return true;
  }         // end if lookup result
  return true;
}

// using the currently defined address, do a DNS
// and try to fill up the name
int IpAddress::addr_to_friendly()
{
  ADDRESS_TRACE;

  // can't look up an invalid address
  if (!valid_flag) return -1;

  struct addrinfo hints,*res = 0;
  int error;
  char ds[MAX_FRIENDLY_NAME];

  strcpy(ds, this->IpAddress::get_printable());
  memset(&hints, 0, sizeof(hints));
  hints.ai_flags = AI_CANONNAME;
#ifdef AI_ADDRCONFIG
  hints.ai_flags |= AI_ADDRCONFIG;
#endif
  error = getaddrinfo(ds, 0, &hints, &res);
  if (error)
  {
    /* errx(1, "%s", gai_strerror(error)); */
    /*NOTREACHED*/
    iv_friendly_name_status = error;
    return 0;
  }
  else
  {
    iv_friendly_name_status = 0;
    if (res->ai_family == AF_INET
        || res->ai_family == AF_INET6
        )
    {
      iv_friendly_name = res->ai_canonname;
      freeaddrinfo(res);
      return 0;
    }
    freeaddrinfo(res);
  }         // end if lookup result
  return -1; //should not get here
}

unsigned int IpAddress::get_scope() const
{
  ADDRESS_TRACE;

  if (valid_flag)
  {
    const unsigned int *scope;
    if ((ip_version == version_ipv6) && (have_ipv6_scope))
      scope = (const unsigned int*)(address_buffer + IP6LEN_NO_SCOPE);
    else
      return (unsigned int)-1;

    return ntohl(*scope);
  }
  return (unsigned int)-1; // don't use uninitialized memory
}

bool IpAddress::set_scope(const unsigned int scope)
{
  ADDRESS_TRACE;

  if (!valid_flag || (ip_version != version_ipv6))
      return false;

  unsigned int *scope_ptr = (unsigned int*)(address_buffer + IP6LEN_NO_SCOPE);

  *scope_ptr = htonl(scope);
  addr_changed = true;
  smival.value.string.len = IP6LEN_WITH_SCOPE;
  have_ipv6_scope = true;
  return true;
}

//----[ IP address format output ]------------------------------------
void IpAddress::format_output() const
{
  ADDRESS_TRACE;

  // if valid format else null it
  if (valid_flag)
  {
    if (ip_version == version_ipv4)
      sprintf((char *) output_buffer,"%d.%d.%d.%d",address_buffer[0],
               address_buffer[1], address_buffer[2], address_buffer[3]);
    else
      if (have_ipv6_scope)
        sprintf((char *) output_buffer,
                "%02x%02x:%02x%02x:%02x%02x:%02x%02x:"
                "%02x%02x:%02x%02x:%02x%02x:%02x%02x%%%d",
                address_buffer[ 0], address_buffer[ 1], address_buffer[ 2],
                address_buffer[ 3], address_buffer[ 4], address_buffer[ 5],
                address_buffer[ 6], address_buffer[ 7], address_buffer[ 8],
                address_buffer[ 9], address_buffer[10], address_buffer[11],
                address_buffer[12], address_buffer[13], address_buffer[14],
                address_buffer[15], get_scope());
      else
        sprintf((char *) output_buffer,
                "%02x%02x:%02x%02x:%02x%02x:%02x%02x:"
                "%02x%02x:%02x%02x:%02x%02x:%02x%02x",
                address_buffer[ 0], address_buffer[ 1], address_buffer[ 2],
                address_buffer[ 3], address_buffer[ 4], address_buffer[ 5],
                address_buffer[ 6], address_buffer[ 7], address_buffer[ 8],
                address_buffer[ 9], address_buffer[10], address_buffer[11],
                address_buffer[12], address_buffer[13], address_buffer[14],
                address_buffer[15]);
  }
  else
    *(char *)output_buffer = 0;
  IpAddress *nc_this = PP_CONST_CAST(IpAddress*, this);
  nc_this->addr_changed = false;
}

//-----------------------------------------------------------------
// logically and two IPaddresses and
// return the new one
void IpAddress::mask(const IpAddress& ipaddr)
{
  ADDRESS_TRACE;

  if (valid() && ipaddr.valid())
  {
    int count = (ip_version == version_ipv4) ? IPLEN : IP6LEN_NO_SCOPE;

    for (int i = 0; i < count; i++)
      address_buffer[i] = address_buffer[i] & ipaddr.address_buffer[i];
    addr_changed = true;
  }
}


// Get the count of matching bits from the left.
int IpAddress::get_match_bits(const IpAddress match_ip) const
{
  ADDRESS_TRACE;

  int bits = 0;

  if (valid() && match_ip.valid() &&
      (ip_version == match_ip.ip_version))
  {
    int count = (ip_version == version_ipv4) ? IPLEN : IP6LEN_NO_SCOPE;

    for (int i = 0; i < count; i++)
    {
        if (address_buffer[i] == match_ip.address_buffer[i])
            bits += 8;
        else
        {
            bits += 7;
            unsigned char c1 = address_buffer[i] >> 1;
            unsigned char c2 = match_ip.address_buffer[i] >> 1;
            while (c1 != c2)
            {
                c1 = c1 >> 1;
                c2 = c2 >> 1;
                bits--;
            }
            break;
        }
    }
  }

  return bits;
}

// Map a IPv4 Address to a IPv6 address.
bool IpAddress::map_to_ipv6()
{
  ADDRESS_TRACE;

  if (!valid())
    return false;

  if (ip_version != version_ipv4)
    return false;

  /* just copy IPv4 address to the end of  the buffer
     zero the first 10 bytes and fill 2 Bytes with 0xff */
  memcpy(&address_buffer[12], address_buffer, 4);
  memset(address_buffer, 0, 10);
  address_buffer[10] = 0xff;
  address_buffer[11] = 0xff;

  smival.value.string.len = IP6LEN_NO_SCOPE;
  ip_version = version_ipv6;
  have_ipv6_scope = false;

  addr_changed = true;
  return true;
}

// Reset the object
void IpAddress::clear()
{
  Address::clear();
  memset(output_buffer, 0, sizeof(output_buffer));
  iv_friendly_name_status = 0;
  ip_version = version_ipv4;
  have_ipv6_scope = false;
  iv_friendly_name.clear();
  smival.value.string.len = IPLEN;
}

//=======================================================================
//========== Udp Address Implementation =================================
//=======================================================================

//-------[ construct an IP address with no agrs ]----------------------
UdpAddress::UdpAddress()
  : IpAddress()
{
  ADDRESS_TRACE;

  // Inherits IP Address attributes
  // Always initialize (override) what type this object is
  smival.syntax = sNMP_SYNTAX_OCTETS;
  smival.value.string.len = UDPIPLEN;
  smival.value.string.ptr = address_buffer;

  sep = ':';
  set_port(0);
}

//-----------------[ construct an Udp address with another Udp address ]---
UdpAddress::UdpAddress(const UdpAddress &udpaddr)
  : IpAddress(udpaddr)
{
  ADDRESS_TRACE;

  // always initialize SMI info
  smival.syntax = sNMP_SYNTAX_OCTETS;
  smival.value.string.len = udpaddr.smival.value.string.len;
  smival.value.string.ptr = address_buffer;

  // Copy the port value
  sep = ':';
  set_port(udpaddr.get_port());

  if (!udpaddr.addr_changed)
  {
    memcpy(output_buffer, udpaddr.output_buffer,
           sizeof(unsigned char) * OUTBUFF);
    addr_changed = false;
  }
}

// constructor with a dotted string
UdpAddress::UdpAddress(const char *inaddr) : IpAddress()
{
  ADDRESS_TRACE;

  // always initialize SMI info
  smival.syntax = sNMP_SYNTAX_OCTETS;
  smival.value.string.len = UDPIPLEN;
  smival.value.string.ptr = address_buffer;

  valid_flag = parse_address(inaddr);
  addr_changed = true;
}

//-----------------[ construct a UdpAddress from a GenAddress ]--------------
UdpAddress::UdpAddress(const GenAddress &genaddr) : IpAddress()
{
  ADDRESS_TRACE;

  // always initialize SMI info
  smival.syntax = sNMP_SYNTAX_OCTETS;
  smival.value.string.len = UDPIPLEN;
  smival.value.string.ptr = address_buffer;

  valid_flag = genaddr.valid();

  // allow use of an ip or udp genaddress
  if (valid_flag)
  {
    if (genaddr.get_type() == type_udp)
    {
      *this = genaddr.cast_udpaddress();      // copy in the IP address data
    }
    else if (genaddr.get_type() == type_ip)
    {
      *this = genaddr.cast_ipaddress();      // copy in the IP address data
    }
    else
    {
      valid_flag = false;
    }
  }
  sep = ':';
}

//--------[ construct a udp from an IpAddress ]--------------------------
UdpAddress::UdpAddress(const IpAddress &ipaddr)
    : IpAddress(ipaddr)
{
  ADDRESS_TRACE;

   // always initialize SMI info
  smival.syntax = sNMP_SYNTAX_OCTETS;
  if (ip_version == version_ipv4)
      smival.value.string.len = UDPIPLEN;
  else
      if (have_ipv6_scope)
          smival.value.string.len = UDPIP6LEN_WITH_SCOPE;
      else
          smival.value.string.len = UDPIP6LEN_NO_SCOPE;
  smival.value.string.ptr = address_buffer;

  sep = ':';
  addr_changed = true;
  set_port(0);
}

// copy an instance of this Value
SnmpSyntax& UdpAddress::operator=(const SnmpSyntax &val)
{
  ADDRESS_TRACE;

  if (this == &val) return *this;   // protect against assignment from itself

  valid_flag = false;                // will get set TRUE if really valid
  addr_changed = true;
  if (val.valid())
  {
    switch (val.get_syntax())
    {
      case sNMP_SYNTAX_IPADDR:
      {
        UdpAddress temp_udp(val.get_printable());
        *this = temp_udp;        // valid_flag is set by the udp assignment
        break;
      }
      case sNMP_SYNTAX_OCTETS:
        if (((UdpAddress &)val).smival.value.string.len == UDPIPLEN)
        {
          memcpy(address_buffer,((UdpAddress &)val).smival.value.string.ptr,
                 UDPIPLEN);
          iv_friendly_name.clear();
          valid_flag = true;
          ip_version = version_ipv4;
          smival.value.string.len = UDPIPLEN;
        }
        else if (((UdpAddress &)val).smival.value.string.len == UDPIP6LEN_NO_SCOPE)
        {
          memcpy(address_buffer,((UdpAddress &)val).smival.value.string.ptr,
                 UDPIP6LEN_NO_SCOPE);
          iv_friendly_name.clear();
          valid_flag = true;
          ip_version = version_ipv6;
          smival.value.string.len = UDPIP6LEN_NO_SCOPE;
          have_ipv6_scope = false;
        }
        else if (((UdpAddress &)val).smival.value.string.len == UDPIP6LEN_WITH_SCOPE)
        {
          memcpy(address_buffer,((UdpAddress &)val).smival.value.string.ptr,
                 UDPIP6LEN_WITH_SCOPE);
          iv_friendly_name.clear();
          valid_flag = true;
          ip_version = version_ipv6;
          smival.value.string.len = UDPIP6LEN_WITH_SCOPE;
          have_ipv6_scope = true;
        }
        break;
        // NOTE: as a value add, other types could have "logical"
        // mappings, i.e. integer32 and unsigned32
    }
  }
  return *this;
}

Address& UdpAddress::operator=(const Address &val)
{
  ADDRESS_TRACE;

  if (this == &val) return *this;   // protect against assignment from itself

  valid_flag = false;                // will get set TRUE if really valid
  addr_changed = true;
  if (val.valid())
  {
    switch (val.get_syntax())
    {
      case sNMP_SYNTAX_IPADDR:
      {
        UdpAddress temp_udp(val.get_printable());
        *this = temp_udp;        // valid_flag is set by the udp assignment
        break;
      }
      case sNMP_SYNTAX_OCTETS:
        if (((UdpAddress &)val).smival.value.string.len == UDPIPLEN)
        {
          MEMCPY(address_buffer,((UdpAddress &)val).smival.value.string.ptr,
                 UDPIPLEN);
          iv_friendly_name.clear();
          valid_flag = true;
          ip_version = version_ipv4;
          smival.value.string.len = UDPIPLEN;
        }
        else if (((UdpAddress &)val).smival.value.string.len == UDPIP6LEN_NO_SCOPE)
        {
          MEMCPY(address_buffer,((UdpAddress &)val).smival.value.string.ptr,
                 UDPIP6LEN_NO_SCOPE);
          iv_friendly_name.clear();
          valid_flag = true;
          ip_version = version_ipv6;
          smival.value.string.len = UDPIP6LEN_NO_SCOPE;
          have_ipv6_scope = false;
        }
        else if (((UdpAddress &)val).smival.value.string.len == UDPIP6LEN_WITH_SCOPE)
        {
          MEMCPY(address_buffer,((UdpAddress &)val).smival.value.string.ptr,
                 UDPIP6LEN_WITH_SCOPE);
          iv_friendly_name.clear();
          valid_flag = true;
          ip_version = version_ipv6;
          smival.value.string.len = UDPIP6LEN_WITH_SCOPE;
          have_ipv6_scope = true;
        }
        break;
        // NOTE: as a value add, other types could have "logical"
        // mappings, i.e. integer32 and unsigned32
    }
  }
  return *this;
}
// assignment to another UdpAddress object overloaded
UdpAddress& UdpAddress::operator=(const UdpAddress &udpaddr)
{
  ADDRESS_TRACE;

  if (this == &udpaddr) return *this; // protect against assignment from itself

  (IpAddress &)*this = udpaddr; // use ancestor assignment for ipaddr value
  if (ip_version == version_ipv4)
    smival.value.string.len = UDPIPLEN;
  else
      if (have_ipv6_scope)
          smival.value.string.len = UDPIP6LEN_WITH_SCOPE;
      else
          smival.value.string.len = UDPIP6LEN_NO_SCOPE;

  set_port(udpaddr.get_port());        // copy to port value
  if (udpaddr.addr_changed)
  {
    addr_changed = true;
  }
  else
  {
    memcpy(output_buffer, udpaddr.output_buffer,
           sizeof(unsigned char) * OUTBUFF);
    addr_changed = false;
  }

  return *this;
}

// assignment to another UdpAddress object overloaded
UdpAddress& UdpAddress::operator=(const IpAddress &ipaddr)
{
  ADDRESS_TRACE;

  if (this == &ipaddr) return *this; // protect against assignment from itself

  IpAddress::operator=(ipaddr); // use ancestor assignment for ipaddr value

  if (ip_version == version_ipv4)
    smival.value.string.len = UDPIPLEN;
  else
      if (have_ipv6_scope)
          smival.value.string.len = UDPIP6LEN_WITH_SCOPE;
      else
          smival.value.string.len = UDPIP6LEN_NO_SCOPE;

  set_port(0);        // copy to port value
  addr_changed = true;
  return *this;
}

//-----[ IP Address parse Address ]---------------------------------
bool UdpAddress::parse_address(const char *inaddr)
{
  ADDRESS_TRACE;

  addr_changed = true;

  char buffer[MAX_FRIENDLY_NAME];

  if (inaddr && (strlen(inaddr)< MAX_FRIENDLY_NAME))
  {
    strcpy(buffer, inaddr);
    trim_white_space(buffer);
  }
  else
  {
    valid_flag = false;
    return false;
  }
  // look for port info @ the end of the string
  // port can be delineated by a ':' or a '/'
  // if neither are present then just treat it
  // like a normal IpAddress

  int remove_brackets = false;
  int found = false;
  int pos = (int)strlen(buffer) - 1; // safe to cast as max is MAX_FRIENDLY_NAME
  int do_loop = true;
  int another_colon_found = false;
  bool scope_found = false;

  if (pos < 0)
  {
    valid_flag = false;
    return false;
  }

  // search from the end, to find the start of the port
  // [ipv4]:port [ipv4]/port ipv4/port ipv4:port [ipv4] ipv4
  // [ipv6]:port [ipv6]/port ipv6/port           [ipv6] ipv6
  while (do_loop)
  {
    if (buffer[pos] == '/')
    {
      found = true;
      sep='/';
      if (buffer[pos -1] == ']')
        remove_brackets = true;
      break;
    }
    if (buffer[pos] == ':')
    {
      if ((pos > 1) && (buffer[pos -1] == ']'))
      {
        found = true;
        remove_brackets = true;
        sep=':';
        break;
      }

      for (int i=pos - 1; i >= 0 ; i--)
      {
          if (buffer[i] == ':')
              another_colon_found = true;
          if (buffer[i] == '%')
              scope_found = true;
      }
      if (scope_found) // must be ipv6, so reset colon_found
          another_colon_found = false;

      if (!another_colon_found)
      {
        sep=':';
        found = true;
        break;
      }
    }
    if (buffer[pos] == ']')
    {
      // we found a ] without following a port, so increase pos
      ++pos;
      remove_brackets = true;
      break;
    }
    pos--;
    do_loop = ((found == false) && (pos >= 0) &&
               (another_colon_found == false));
  }

  if (remove_brackets)
  {
    buffer[pos-1] = 0;
    buffer[0] = ' ';
  }

  bool result;
  unsigned short port;

  if (found)
  {
    buffer[pos] = 0;
    port = atoi(&buffer[pos+1]);
    result = IpAddress::parse_address(buffer);
  }
  else
  {
    port = 0;
    result = IpAddress::parse_address(buffer);
  }

  if (ip_version == version_ipv4)
    smival.value.string.len = UDPIPLEN;
  else
      if (have_ipv6_scope)
          smival.value.string.len = UDPIP6LEN_WITH_SCOPE;
      else
          smival.value.string.len = UDPIP6LEN_NO_SCOPE;

  set_port(port);
  return result;
}


//--------[ set the port number ]---------------------------------------
void UdpAddress::set_port(const unsigned short p)
{
  ADDRESS_TRACE;

  unsigned short *port_nbo;
  if (ip_version == version_ipv4)
    port_nbo = (unsigned short*)(address_buffer + IPLEN);
  else
      if (have_ipv6_scope)
          port_nbo = (unsigned short*)(address_buffer + IP6LEN_WITH_SCOPE);
      else
          port_nbo = (unsigned short*)(address_buffer + IP6LEN_NO_SCOPE);
  *port_nbo = htons(p);
  addr_changed = true;
}

//---------[ get the port number ]--------------------------------------
unsigned short UdpAddress::get_port() const
{
  ADDRESS_TRACE;

  if (valid_flag)
  {
    const unsigned short *port_nbo;
    if (ip_version == version_ipv4)
      port_nbo = (const unsigned short*)(address_buffer + IPLEN);
    else
        if (have_ipv6_scope)
            port_nbo = (const unsigned short*)(address_buffer + IP6LEN_WITH_SCOPE);
        else
            port_nbo = (const unsigned short*)(address_buffer + IP6LEN_NO_SCOPE);

    return ntohs(*port_nbo);
  }
  return 0;// don't use uninitialized memory
}

//----[ UDP address format output ]------------------------------------
void UdpAddress::format_output() const
{
  ADDRESS_TRACE;

  IpAddress::format_output(); // allow ancestors to format their buffers

  // if valid format else null it
  if (valid_flag)
  {
    if (ip_version == version_ipv4)
      sprintf((char *) output_buffer,"%s%c%d",
               IpAddress::get_printable(),
               '/',//TODO:look for problems in old code and change to "sep"
               get_port() );
      else
        sprintf((char *) output_buffer,"[%s]%c%d",
                 IpAddress::get_printable(),
                 '/',//TODO:look for problems in old code and change to "sep"
                 get_port() );
  }
  else
    *(char*)output_buffer = 0;
  UdpAddress *nc_this = PP_CONST_CAST(UdpAddress*, this);
  nc_this->addr_changed = false;
}

bool UdpAddress::set_scope(const unsigned int scope)
{
  ADDRESS_TRACE;

  /* Save the port, as IpAddress::set_scope destroys it */
  unsigned short old_port = get_port();

  if (!IpAddress::set_scope(scope))
      return false;

  smival.value.string.len = UDPIP6LEN_WITH_SCOPE;

  set_port(old_port);

  return true;
}

/**
 * Map a IPv4 UDP address to a IPv6 UDP address.
 *
 * @return - TRUE if no error occured.
 */
bool UdpAddress::map_to_ipv6()
{
  ADDRESS_TRACE;

  /* Save the port, as IpAddress::map_to_ipv6 destroys it */
  unsigned short old_port = get_port();

  /* Map IpAddress */
  if (!IpAddress::map_to_ipv6())
    return false;

  set_port(old_port);
  smival.value.string.len = UDPIP6LEN_NO_SCOPE;
  ip_version = version_ipv6;

  addr_changed = true;
  return true;
}

//========================================================================
//========== Generic Address Implementation ==============================
//========================================================================

//-----------[ constructor, no arguments ]--------------------------------
GenAddress::GenAddress() : Address()
{
  ADDRESS_TRACE;

  // initialize SMI info
  // BOK: this is generally not used for GenAddress,
  // but we need this to be a replica of the real address'
  // smival info so that operator=SnmpSyntax will work.
  smival.syntax = sNMP_SYNTAX_NULL;                // to be overridden
  smival.value.string.len = 0;                        // to be overridden
  smival.value.string.ptr = address_buffer;        // constant

  valid_flag = false;
  address = 0;
  output_buffer[0] = 0;
}

//-----------[ constructor with a string argument ]----------------------
GenAddress::GenAddress(const char  *addr,
                       const Address::addr_type use_type)
{
  ADDRESS_TRACE;

  // initialize SMI info
  // BOK: smival is generally not used for GenAddress, but
  //      we need this to be a replica of the real address'
  //      smival info so that <class>::operator=SnmpSyntax
  //      will work.
  smival.syntax = sNMP_SYNTAX_NULL;                // to be overridden
  smival.value.string.len = 0;                        // to be overridden
  smival.value.string.ptr = address_buffer;        // constant

  address = 0;
  parse_address(addr, use_type);

  // Copy real address smival info into GenAddr smival
  // BOK: smival is generally not used for GenAddress, but
  //      we need this to be a replica of the real address'
  //      smival info so that <class>::operator=SnmpSyntax
  //      will work.
  if (valid_flag ) {
      smival.syntax = ((GenAddress *)address)->smival.syntax;
      smival.value.string.len =
          ((GenAddress *)address)->smival.value.string.len;
      memcpy(smival.value.string.ptr,
          ((GenAddress *)address)->smival.value.string.ptr,
          (size_t)smival.value.string.len);
  }
  output_buffer[0] = 0;
}

//-----------[ constructor with an Address argument ]--------------------
GenAddress::GenAddress(const Address &addr)
{
  ADDRESS_TRACE;

  output_buffer[0] = 0;
  // initialize SMI info
  // BOK: this is generally not used for GenAddress,
  // but we need this to be a replica of the real address'
  // smival info so that operator=SnmpSyntax will work.
  smival.syntax = sNMP_SYNTAX_NULL;                // to be overridden
  smival.value.string.len = 0;                        // to be overridden
  smival.value.string.ptr = address_buffer;        // constant

  valid_flag = false;
  // make sure that the object is valid
  if (!addr.valid()) {
    address = 0;
    return;
  }

  // addr can be a GenAddress object and calling clone() on that is bad...
  if (addr.is_gen_address())
    address = (Address *)(((const GenAddress&)addr).address->clone());
  else
    address = (Address*)addr.clone();

  if (address)
    valid_flag = address->valid();

  // Copy real address smival info into GenAddr smival
  // BOK: smival is generally not used for GenAddress, but
  //      we need this to be a replica of the real address'
  //      smival info so that <class>::operator=SnmpSyntax
  //      will work.
  if (valid_flag )
  {
    smival.syntax = address->get_syntax();
    smival.value.string.len = ((GenAddress *)address)->smival.value.string.len;
    memcpy(smival.value.string.ptr,
           ((GenAddress *)address)->smival.value.string.ptr,
           (size_t)smival.value.string.len);
  }
}

//-----------------[ constructor with another GenAddress object ]-------------
GenAddress::GenAddress(const GenAddress &addr)
{
  ADDRESS_TRACE;

  output_buffer[0] = 0;
  // initialize SMI info
  // BOK: this is generally not used for GenAddress,
  // but we need this to be a replica of the real address'
  // smival info so that operator=SnmpSyntax will work.
  smival.syntax = sNMP_SYNTAX_OCTETS;
  smival.value.string.len = 0;
  smival.value.string.ptr = address_buffer;

  valid_flag = false;
  // make sure that the object is valid
  if (!addr.valid_flag)
  {
    address = 0;
    return;
  }

  address = (Address *)addr.address->clone();
  if (address)
    valid_flag = address->valid();

  // Copy real address smival info into GenAddr smival
  // BOK: smival is generally not used for GenAddress, but
  //      we need this to be a replica of the real address'
  //      smival info so that <class>::operator=SnmpSyntax
  //      will work.
  if (valid_flag )
  {
    smival.syntax = ((GenAddress *)address)->smival.syntax;
    smival.value.string.len = ((GenAddress *)address)->smival.value.string.len;
    memcpy(smival.value.string.ptr,
           ((GenAddress *)address)->smival.value.string.ptr,
           (size_t)smival.value.string.len);
  }
}

//------[ assignment GenAddress = GenAddress ]-----------------------------
GenAddress& GenAddress::operator=(const GenAddress &addr)
{
  ADDRESS_TRACE;

  if (this == &addr) return *this;  // protect against assignment from itself

  valid_flag = false;
  if (address)
  {
    delete address;
    address = 0;
  }
  if (addr.address)
    address = (Address *)(addr.address->clone());
  if (address)
    valid_flag = address->valid();

  // Copy real address smival info into GenAddr smival
  // BOK: smival is generally not used for GenAddress, but
  //      we need this to be a replica of the real address'
  //      smival info so that <class>::operator=SnmpSyntax
  //      will work.
  if (valid_flag )
  {
    smival.syntax = ((GenAddress *)address)->smival.syntax;
    smival.value.string.len = ((GenAddress *)address)->smival.value.string.len;
    memcpy(smival.value.string.ptr,
           ((GenAddress *)address)->smival.value.string.ptr,
           (size_t)smival.value.string.len);
  }

  return *this;
}

//------[ assignment GenAddress = Address ]--------------------------------
Address& GenAddress::operator=(const Address &addr)
{
  ADDRESS_TRACE;

  if (this == &addr) return *this;  // protect against assignment from itself

  valid_flag = false;
  if (address)
  {
    delete address;
    address = 0;
  }

  // addr can be a GenAddress object and calling clone() on that is bad...
  if (addr.is_gen_address())
    address = (Address *)(((const GenAddress&)addr).address->clone());
  else
    address = (Address*)addr.clone();

  if (address)
    valid_flag = address->valid();

  // Copy real address smival info into GenAddr smival
  // BOK: smival is generally not used for GenAddress, but
  //      we need this to be a replica of the real address'
  //      smival info so that <class>::operator=SnmpSyntax
  //      will work.
  if (valid_flag )
  {
    smival.syntax = ((GenAddress *)address)->smival.syntax;
    smival.value.string.len = ((GenAddress *)address)->smival.value.string.len;
    memcpy(smival.value.string.ptr,
           ((GenAddress *)address)->smival.value.string.ptr,
           (size_t)smival.value.string.len);
  }

  return *this;
}


//------[ assignment GenAddress = any SnmpSyntax ]-----------------------
SnmpSyntax& GenAddress::operator=(const SnmpSyntax &val)
{
  ADDRESS_TRACE;

  if (this == &val) return *this; // protect against assignment from itself

  valid_flag = false;             // will get set to TRUE if really valid
  if (address)
  {
    delete address;
    address = 0;
  }

  if (val.valid())
  {
    switch (val.get_syntax() )
    {
      //-----[ ip address case ]-------------
      // BOK: this case shouldn't be needed since there is an explicit
      // GenAddr=Address assignment that will override this assignment.
      // Left here for posterity.
      case sNMP_SYNTAX_IPADDR:
      {
        address = new IpAddress(val.get_printable());
        if (address)
          valid_flag = address->valid();
      }
      break;

      //-----[ udp address case ]------------
      //-----[ ipx address case ]------------
      //-----[ mac address case ]------------
      // BOK:  This is here only to support GenAddr = primitive OctetStr.
      // The explicit GenAddr=Address assignment will handle the cases
      // GenAddr = [UdpAdd|IpxAddr|IpxSock|MacAddr].
      // Note, using the heuristic of octet str len to determine type of
      // address to create is not accurate when address lengths are equal
      // (e.g., UDPIPLEN == MACLEN).  It gets worse if we add AppleTalk or
      // OSI which use variable length addresses!
      case sNMP_SYNTAX_OCTETS:
      {
        unsigned long val_len;
        val_len = ((GenAddress &)val).smival.value.string.len;

        if ((val_len == UDPIPLEN) || IS_UDPIP6LEN(val_len))
          address = new UdpAddress;
        else if ((val_len == IPLEN) || IS_IP6LEN(val_len))
          address = new IpAddress;

        if (address)
        {
          *address = val;
          valid_flag = address->valid();
        }
      }
      break;
    }   // end switch
  }

  // Copy real address smival info into GenAddr smival
  // BOK: smival is generally not used for GenAddress, but
  //      we need this to be a replica of the real address'
  //      smival info so that <class>::operator=SnmpSyntax
  //      will work.
  if (valid_flag )
  {
    smival.syntax = ((GenAddress *)address)->smival.syntax;
    smival.value.string.len = ((GenAddress *)address)->smival.value.string.len;
    memcpy(smival.value.string.ptr,
           ((GenAddress *)address)->smival.value.string.ptr,
           (size_t)smival.value.string.len);
  }

  return *this;
}


// redefined parse address for macs
bool GenAddress::parse_address(const char *addr,
                               const Address::addr_type use_type)
{
  ADDRESS_TRACE;

  if (address) delete address;

  // try to create each of the addresses until the correct one
  // is found

  //BOK: Need to try IPX Sock and IPX before UDP since on Win32,
  //     gethostbyname() seems to think the ipx network number
  //     portion is a valid ipaddress string... stupid WinSOCK!


  //TM: Must try the derived classes first...one pitfall of the
  //following solution is if someone creates with a port/socket of 0 the
  //class will get demoted to ip/ipx.  The only proper way to do this is
  //to parse the strings ourselves.

  if ((use_type == Address::type_invalid) ||
      (use_type == Address::type_udp))
  {
    // udp address
    address = new UdpAddress(addr);
    valid_flag = address->valid();
    if (valid_flag && ((UdpAddress*)address)->get_port())
      return true;       // ok its a udp address

    delete address;  // otherwise delete it and try another
  }

  if ((use_type == Address::type_invalid) ||
      (use_type == Address::type_ip))
  {
    // ip address
    address = new IpAddress(addr);
    valid_flag = address->valid();
    if (valid_flag)
      return true;       // ok its an ip address

    delete address;   // otherwise delete it and try another
  }

  address = 0;
  return false;
}

} // end of namespace Snmp_pp
