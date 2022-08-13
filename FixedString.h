// Fixed String
// (c) Copyright 2022 Fatlab Software Pty Ltd.
//
//This library is free software; you can redistribute it and/or
//modify it under the terms of the GNU Lesser General Public
//License as published by the Free Software Foundation; either
//version 2.1 of the License, or (at your option) any later version.
//
//This library is distributed in the hope that it will be useful,
//but WITHOUT ANY WARRANTY; without even the implied warranty of
//MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.See the GNU
//Lesser General Public License for more details.
//
//You should have received a copy of the GNU Lesser General Public
//License along with this library; if not, write to the Free Software
//Foundation, Inc., 51 Franklin St, Fifth Floor, Boston, MA  02110 - 1301  USA

#ifndef _FIXEDSTRING_H
#define _FIXEDSTRING_H
#pragma once
#ifdef ARDUINO
#include "Arduino.h"
#include "WString.h"

#else //Testing 
#if __cplusplus < 201703L
#error Require C++17 for non-Arduino testing!
#endif
#include <stdlib.h>
#include <string.h>
#include <ctype.h>
#include <charconv> //Assumes c++17+ for Non Arduino usage/testing
#endif

#ifdef ARDUINO
///////////////////////////////////////////////////////////////////////////////////////////
// Flash memory Helpers
//NB. variables must be globally defined, OR defined with the static keyword, to work with PROGMEM.
//  The following code will NOT work when inside a function:
//  const char long_str[] PROGMEM = "Hi, I would like to tell you a bit about myself.\n";
//  The following code WILL work, even if locally defined within a function:
//  const static char long_str[] PROGMEM = "Hi, I would like to tell you a bit about myself.\n"
//
//To define Flash String constant use DEFINE_PSTR()
//
// DEFINE_PSTR(my_const_str_val,"This is a literal string held completely in flash memory."); 
//
// To then access this string use GET_PSTR()
//
// String s(GET_PSTR(my_const_str_val));
//
//NB. Use F() for literal strings
//
// String s(F("My flash string"));
//
#define DEFINE_PSTR(lbl,s)	static const char pstr_##lbl[] PROGMEM = s
#define GET_PSTR(lbl)	(reinterpret_cast<const __FlashStringHelper*>(pstr_##lbl))
#else 
#define DEFINE_PSTR(lbl,s)	static const char pstr_##lbl[] = s 
#define GET_PSTR(lbl)	pstr_##lbl
#endif

///////////////////////////////////////////////////////////////////////////////////////////
//  Radix
//  used for number to string conversion
///////////////////////////////////////////////////////////////////////////////////////////
enum Radix { base2 = 2, base8 = 8, base10 = 10, base16 = 16 };

///////////////////////////////////////////////////////////////////////////////////////////
// FixedString
// A wrapper class around a stack based fixed string char my_str[c_storage_size];
// The length of the string is cached in a byte (Thus the 256 byte limit!) to speed up operations.
// c_storage_size:  is the total bytes used for this class
//					it must be >=4, <= 256 and divisible by 4
//					the default == 64
// It is best used on the stack as a drop-in replacement for String to avoid dynamic memory allocation
// for smaller strings
///////////////////////////////////////////////////////////////////////////////////////////
template<unsigned int c_storage_size = 64>
class FixedString final
{
	static const unsigned int c_min_storage = 4;
	static const unsigned int c_max_storage = 256;
	//Min == 4 bytes Need 1 length AND 1 for zero byte so 2 char string is smallest allowed!
	static_assert(c_storage_size >= c_min_storage && c_storage_size <= c_max_storage, "Must be between between 4 & 256");
	static_assert(c_storage_size % 4 == 0, "Must be divisible by 4");//Force 4 byte boundary 
public:
	using char_type = char;
	using value_type = char_type;
	using pointer = value_type*;
	using const_pointer = const value_type*;
	using size_type = unsigned int;
#ifdef ARDUINO
	using FlashPtr = const __FlashStringHelper*;
#endif
	static constexpr const size_type npos = (size_type)-1;
	//Need 2 non-data chars: 1 for length AND 1 for zero end char
	static constexpr const size_type capacity() { return c_storage_size - 2; }
private:
	//Helpers
	static constexpr const size_type get_min(size_type i1, size_type i2) { return i1 < i2 ? i1 : i2; }
	static constexpr const size_type get_max(size_type i1, size_type i2) { return i1 > i2 ? i1 : i2; }
	static constexpr bool valid_len(size_type len) { return len <= capacity(); }
	static constexpr const size_type safe_len(const_pointer str)
	{
		return str == NULL ? 0u : static_cast<size_type>(strlen(str));
	}
	static constexpr bool is_valid(const_pointer data, size_type len)
	{
		return data != NULL || len == 0u;//allow empty
	}
	static constexpr bool is_empty(const_pointer data, size_type len)
	{
		return data == NULL || len == 0u;
	}
private:
	// The Length limited to [0 .. c_storage_size-2]
	//	We store to speed up string operations
	unsigned char m_len;
	//The actual string data
	char_type m_str[c_storage_size - 1];
public:
	FixedString() :m_len(0u), m_str{ 0 } {} //Make sure null added
	FixedString(const_pointer str) { assign(str); }
	FixedString(const_pointer lpch, size_type len) { assign(lpch, len); }
	FixedString(char_type c, size_type repeat = 1) { assign(repeat, c); }
	template<size_type c_storage_size2>
	FixedString(const FixedString<c_storage_size2>& rhs) { assign(rhs); }
	//Numeric
	//Numeric Set
	explicit FixedString(char i, Radix r) { assign(i, r); }
	explicit FixedString(unsigned char u, Radix r) { assign(u, r); }
	explicit FixedString(int i, Radix r = base10) { assign(i, r); }
	explicit FixedString(unsigned int u, Radix r = base10) { assign(u, r); }
	explicit FixedString(long l, Radix r = base10) { assign(l, r); }
	explicit FixedString(unsigned long u, Radix r = base10) { assign(u, r); }
	explicit FixedString(float f, size_type decPlaces = 2) { assign(f, decPlaces); }
	explicit FixedString(double d, size_type decPlaces = 2) { assign(d, decPlaces); }
	//overloaded assignment
	FixedString& operator=(const_pointer str) { assign(str); return *this; }
	FixedString& operator=(char_type c) { assign(c); return *this; }
	template<size_type c_storage_size2>
	FixedString& operator=(const FixedString<c_storage_size2>& rhs) { assign(rhs); return *this; }
	//Numeric
	template<typename Num>
	FixedString& operator=(Num num) { assign(num); return *this; }
#ifdef ARDUINO
	FixedString(FlashPtr str) :FixedString() { assign(str); }
	FixedString(const String& s) :FixedString(s.c_str(), s.length()) {}
	FixedString& operator=(FlashPtr str) { assign(str); return *this; }
	FixedString& operator=(const String& s) { assign(s); return *this; }
#endif
public:
	//The string length - we return local cached value for speed
	size_type length()const { return m_len; }
	//No of free chars available for concat
	size_type available()const { return capacity() - length(); }
	bool full()const { return available() == 0u; }
	bool empty()const { return length() == 0u; }

public:
	//set to a valid empty string
	void clear() { set_len(0); }
	//assign
	bool assign(const_pointer data, size_type len) { clear(); return concat(data, len); }
	bool assign(const_pointer str) { return assign(str, safe_len(str)); }
	bool assign(size_type repeat, char_type c) { clear(); return concat(repeat, c); }
	template<size_type c_storage_size2>
	bool assign(const FixedString<c_storage_size2>& rhs) { clear(); return concat(rhs); }
#ifdef ARDUINO
	bool assign(const String& s) { clear(); return concat(s); }
	bool assign(FlashPtr str)
	{
		clear();
		const_pointer prog_ptr = reinterpret_cast<const_pointer>(str);
		if (prog_ptr == NULL)
			return false;
		const auto prog_len = strlen_P(prog_ptr);
		const auto actual_cnt = get_min(available(), prog_len);
		memcpy_P(begin(), prog_ptr, actual_cnt);//copy data
		return set_len(actual_cnt, actual_cnt == prog_len);
	}
#endif
	bool assign(char i, Radix r) { return set_i(i, r); }
	bool assign(unsigned char u, Radix r) { return set_u(u, r); }
	bool assign(int i, Radix r = base10) { return set_i(i, r); }
	bool assign(unsigned int u, Radix r = base10) { return set_u(u, r); }
	bool assign(long i, Radix r = base10) { return set_l(i, r); }
	bool assign(unsigned long u, Radix r = base10) { return set_ul(u, r); }
	bool assign(float f, size_type decPlaces = 2) { return set_f(f, decPlaces); }
	bool assign(double d, size_type decPlaces = 2) { return set_f(d, decPlaces); }
	//concat
	bool concat(const_pointer data, size_type len) { return handle_insert(length(), data, len); }
	bool concat(const_pointer str) { return handle_insert(length(), str); }
	bool concat(char_type c) { return handle_insert(length(), 1, c); }
	bool concat(size_type repeat, char_type c) { return handle_insert(length(), repeat, c); }
	template<size_type c_storage_size2>
	bool concat(const FixedString<c_storage_size2>& rhs) { return concat(rhs.begin(), rhs.length()); }
	//Numeric float
	bool concat(double d, size_type decPlaces = 2) { return concat(FixedString(d, decPlaces)); }
	//Numeric integer
	template<typename Num>
	bool concat(Num n, Radix r = base10) { return concat(FixedString(n, r)); }
	//Special case - overwrite end char if full, otherwise just add
	bool force_concat(char_type c)
	{
		if (!full())
			return concat(c);
		m_str[length() - 1] = c;//Overwrite current end char
		return true;
	}
	template<typename Param>
	FixedString& operator+=(Param p) { concat(p); return *this; }
	template<size_type c_storage_size2>
	FixedString& operator+=(const FixedString& s) { concat(s); return *this; }
#ifdef ARDUINO
	bool concat(const String& s) { return handle_insert(length(), s.c_str(), s.length()); }
	bool concat(FlashPtr str) { return concat(FixedString(str)); }
	FixedString& operator+=(const String& s) { concat(s); return *this; }
	FixedString& operator+=(FlashPtr str) { concat(str); return *this; }
#endif
	//Support for C style sprintf format
	void format(const_pointer fmt, ...)
	{
		clear();
		if (safe_len(fmt) == 0)
			return;
		va_list args;
		va_start(args, fmt);
		formatV(fmt, args);
		va_end(args);
	}

	//string comparison
	int compareTo(const FixedString& rhs)const
	{
		return (&rhs == this)
			? 0
			: compareTo(rhs.c_str(), rhs.length());
	}
	template<int c_storage_size2>
	int compareTo(const FixedString<c_storage_size2>& rhs)const { return compareTo(rhs.c_str(), rhs.length()); }
	int compareTo(const_pointer rhs)const { return compareTo(rhs, safe_len(rhs)); }
	int compareTo(const_pointer rhs, size_type len)const
	{
		if (is_empty(rhs, len))
			return empty() ? 0 : charAt(0);
		if (empty())
			return -rhs[0];
		return strncmp(c_str(), rhs, get_max(length(), len));
	}
	bool operator<(const FixedString& rhs)const { return compareTo(rhs) < 0; }
	bool operator>(const FixedString& rhs)const { return compareTo(rhs) > 0; }
	bool operator<=(const FixedString& rhs)const { return compareTo(rhs) <= 0; }
	bool operator>=(const FixedString& rhs)const { return compareTo(rhs) >= 0; }

	bool operator<(const_pointer rhs)const { return compareTo(rhs) < 0; }
	bool operator>(const_pointer rhs)const { return compareTo(rhs) > 0; }
	bool operator<=(const_pointer rhs)const { return compareTo(rhs) <= 0; }
	bool operator>=(const_pointer rhs)const { return compareTo(rhs) >= 0; }

	//equals
	bool equals(const_pointer rhs, size_type len)const { return equals(rhs, len, false); }
	template<size_type c_storage_size2>
	bool equals(const FixedString& rhs)const { return equals(rhs, false); }
	bool equals(const_pointer rhs)const { return equals(rhs, false); }

	bool operator==(const_pointer rhs)const { return equals(rhs, false); }
	bool operator!=(const_pointer rhs)const { return !equals(rhs, false); }

	bool operator==(const FixedString& rhs)const { return equals(rhs, false); }
	bool operator!=(const FixedString& rhs)const { return !equals(rhs, false); }
	template<size_type c_storage_size2>
	bool operator==(const FixedString<c_storage_size2>& rhs)const { return equals(rhs); }
	template<size_type c_storage_size2>
	bool operator!=(const FixedString<c_storage_size2>& rhs)const { return !equals(rhs); }

	bool equalsIgnoreCase(const FixedString& rhs)const { return equals(rhs, true); }
	bool equalsIgnoreCase(const_pointer rhs)const { return equals(rhs, true); }

	//Data access
	char_type charAt(size_type index)const { return valid_pos(index) ? m_str[index] : 0; }
	char_type operator[](size_type index)const { return charAt(index); }
	void setCharAt(size_type index, char_type c)
	{
		if (!valid_pos(index))
			return;
		m_str[index] = c;
		if (c == '\0')
			set_len(index);//shrink
	}

	void getBytes(unsigned char* buf, size_type bufsize, size_type index = 0) const
	{
		toCharArray((char*)buf, bufsize, index);
	}
	void toCharArray(char* buf, size_type bufsize, size_type index = 0) const
	{
		if (is_empty(buf, bufsize))
			return;
		if (index >= length())
		{
			buf[0] = '\0';
			return;
		}
		auto n = bufsize - 1;
		if (n > length() - index)
			n = length() - index;
		memcpy(buf, c_str() + index, n);
		buf[n] = 0;
	}
	operator const_pointer()const { return m_str; }
	const_pointer c_str()const { return m_str; }
	const_pointer begin()const { return m_str; }
	const_pointer end()const { return begin() + length(); }
	pointer begin() { return m_str; }
	pointer	end() { return begin() + length(); }
	//Call if string modified externally via begin()/end()
	void update_len() { set_len(get_min(safe_len(c_str()), capacity())); }

	//search
	bool startsWith(const_pointer s, size_type offset = 0)const
	{
		const auto rhs_len = safe_len(s);
		if (rhs_len == 0 || (offset + rhs_len) > length())
			return false;
		return strncmp(data_offset(offset), s, rhs_len) == 0;
	}
	bool endsWith(const_pointer s)const
	{
		const auto rhs_len = safe_len(s);
		if (rhs_len == 0 || rhs_len > length())
			return false;
		return strncmp(data_offset(length() - rhs_len), s, rhs_len) == 0;
	}
	int indexOf(char_type c, size_type start_pos = 0)const
	{
		if (!valid_pos(start_pos))
			return -1;
		const_pointer p = strchr(data_offset(start_pos), c);
		if (p == NULL)
			return -1;
		return static_cast<int>(p - begin());
	}
	int indexOf(const_pointer s, size_type start_pos = 0)const
	{
		if (!valid_pos(start_pos))
			return -1;
		const auto rhs_len = safe_len(s);
		if (rhs_len == 0 || (start_pos + rhs_len) > length())
			return -1;
		const_pointer p = strstr(data_offset(start_pos), s);
		if (p == NULL)
			return -1;
		return static_cast<int>(p - begin());
	}
	int lastIndexOf(char_type c)const
	{
		if (empty() || c == 0)
			return -1;
		const_pointer p = strrchr(c_str(), c);
		if (p == NULL)
			return -1;
		return static_cast<int>(p - begin());
	}
	int lastIndexOf(char_type c, size_type from_pos)const
	{
		if (from_pos >= length())
			return lastIndexOf(c);
		if (empty() || c == 0)
			return -1;
		return substring(0, from_pos + 1).lastIndexOf(c);
	}
	int lastIndexOf(const_pointer s, size_type from_pos = npos)const
	{
		if (empty())
			return -1;
		const auto rhs_len = safe_len(s);
		if (rhs_len == 0 || rhs_len > length())
			return -1;
		if (from_pos >= length())
			from_pos = length() - 1;//end
		int found = -1;
		for (const_pointer p = begin(); p <= data_offset(from_pos); p++)
		{
			p = strstr(p, s);
			if (p == NULL)
				break;
			auto index = static_cast<int>(p - begin());
			if (static_cast<size_type>(index) <= from_pos)
				found = index;
		}
		return found;
	}
	FixedString substring(size_type left, size_type right = npos)const
	{
		if (right == npos)
			right = length();
		else if (left > right)
		{
			auto temp = right;
			right = left;
			left = temp;
		}
		if (left >= length())
			return FixedString();//empty
		if (right > length())
			right = length();
		return FixedString(data_offset(left), right - left);
	}

public:
	void replace(char_type c, char_type new_c)
	{
		if (c == 0 || new_c == 0 || c == new_c)
			return;
		for (char_type& p : *this)
		{
			if (p == c)
				p = new_c;
		}
	}
	void replace(const_pointer s, const_pointer new_s)
	{
		if (empty())
			return;
		auto src_len = safe_len(s);
		if (src_len == 0)
			return;
		auto repl_len = safe_len(new_s);
		//Drop out if same
		if (src_len == repl_len && strcmp(s, new_s) == 0)
			return;
		for (auto index = indexOf(s); valid_pos(index); index = indexOf(s, index))
		{
			if (repl_len > 0)
			{
				handle_replace(index, src_len, new_s, repl_len);
				index += repl_len;	//Move to end of this replace string ready for next search
			}
			else
				remove(index, src_len);
			if (!valid_pos(index))//Drop out if index over the end
				break;
		}
	}
	void remove(size_type index) { if (index < length()) set_len(index); }
	void remove(size_type index, size_type cnt)
	{
		if (cnt == 0)
			return;
		if (!valid_pos(index))
			return;
		const auto max_remove_cnt = length() - index;
		auto actual_cnt = get_min(cnt, max_remove_cnt);//shrink to max available cnt
		memmove(begin() + index, begin() + index + actual_cnt, max_remove_cnt - actual_cnt + 1);//shift rem chars down (including null char)
		set_len(length() - actual_cnt);
	}
	void insert(size_type index, const_pointer str) { handle_insert(index, str); }
	void insert(size_type index, const_pointer data, size_type len) { handle_insert(index, data, len); }
	void insert(size_type index, size_type repeat, char_type c) { handle_insert(index, repeat, c); }
	void toLowerCase()
	{
		for (char_type& p : *this)
			p = static_cast<char_type>(tolower(p));
	}
	void toUpperCase()
	{
		for (char_type& p : *this)
			p = static_cast<char_type>(toupper(p));
	}
	void trim()
	{
		if (empty())
			return;
		const_pointer start = begin();
		const_pointer last = end() - 1;
		while (isspace(*start))
			start++;
		while (isspace(*last) && last >= start)last--;
		if (start > begin())
			remove(0, static_cast<size_type>(start - begin()));
		set_len(static_cast<size_type>(last - start + 1));
	}
public:
	//Numeric parsing
	long	toInt()const { return to_int<long>(); }
	float	toFloat()const { return to_float<float>(); }
	double	toDouble()const { return to_float<double>(); }
	template<typename T = double>
	auto to_float()const -> T
	{
		T d = 0.0f;
		get_float(d);
		return d;
	}
	template<typename T = double>
	bool get_float(T& d)const
	{
		d = 0.0f;
		if (empty())
			return false;
		d = static_cast<T>(::atof(c_str()));
		return !isnan(d);
	}
	template<typename T = int>
	auto to_int()const -> T
	{
		T i = 0;
		get_int(i);
		return i;
	}
	template<typename T = int>
	bool get_int(T& i)const
	{
		i = 0;
		if (empty())
			return false;
		i = static_cast<T>(::atol(c_str()));
		return true;
	}

private:
	//helpers
	bool valid_pos(size_type index)const { return index < length(); }
	const_pointer data_offset(size_type off)const { return begin() + off; }
	bool handle_insert(size_type index, const_pointer str, bool allowPartial = true)
	{
		return handle_insert(index, str, safe_len(str), allowPartial);
	}
	bool handle_insert(size_type index, const_pointer data, size_type len, bool allowPartial = true)
	{
		if (index > length())
			return false;
		if (is_empty(data, len))
			return false;
		if (full())
			return notify_overrun(data, len);
		if (!allowPartial && len > capacity())
			return notify_overrun(data, len);

		const auto actual_cnt = get_min(available(), len);
		memmove(begin() + index + actual_cnt, data_offset(index), length() - index);//shift rem chars up (including null char)
		memcpy(begin() + index, data, actual_cnt);//copy data
		return set_len(length() + actual_cnt, actual_cnt == len);
	}
	bool handle_insert(size_type index, size_type repeat, char_type c)
	{
		if (index > length())
			return false;
		if (repeat == 0 || c == '\0')
			return false;
		if (full())
			return notify_overrun();
		const auto actual_cnt = get_min(available(), repeat);
		memmove(begin() + index + actual_cnt, data_offset(index), length() - index);//shift rem chars up (including null char)
		for (auto i = 0u; i < actual_cnt; i++)
			m_str[index + i] = c;
		return set_len(length() + actual_cnt, actual_cnt == repeat);
	}
	void handle_replace(size_type index, size_type erase_cnt, const_pointer lpch, size_type len)
	{
		if (erase_cnt > 0)
			remove(index, erase_cnt);
		handle_insert(index, lpch, len);
	}

	void set_len(size_type len)
	{
		if (!valid_len(len))
			return;
		m_len = static_cast<uint8_t>(len);
		m_str[m_len] = 0;
	}
	bool set_len(size_type len, bool append_ok)
	{
		set_len(len);
		return append_ok || notify_overrun();
	}
	bool equals(const FixedString& rhs, bool b_insens)const
	{
		return (&rhs == this) ? true : equals(rhs.c_str(), rhs.length(), b_insens);
	}
	bool equals(const_pointer rhs, bool b_insens)const { return equals(rhs, safe_len(rhs), b_insens); }
	bool equals(const_pointer rhs, size_type len, bool b_insens)const
	{
		if (length() != len)
			return false;
		if (!is_valid(rhs, len))
			return false;
		if (!b_insens)
			return compareTo(rhs, len) == 0;
		//case insensitive
		for (auto i = 0u; i < len; i++)
			if (toupper(charAt(i)) != toupper(rhs[i]))
				return false;
		return true;
	}
	void formatV(const_pointer fmt, va_list args)
	{
		if (safe_len(fmt) == 0)
			return;
		va_list argssave;
		va_copy(argssave, args);
		auto ret = vsnprintf(m_str, capacity() + 1, fmt, args);
		va_end(argssave);
		if (ret < 0)
		{
			clear(); //error
			return;
		}
		auto len = static_cast<size_type>(ret);
		if (len >= 0 && len <= capacity())
			set_len(len);
		else if (len > capacity())
			set_len(capacity(), false);//Assume overflow so truncate
	}

#ifndef ARDUINO
	//Non Arduino code simply uses std::to_chars Assumes c++17!
	template<typename TInt>
	bool set_chars(TInt i, Radix r = base10)
	{
		//We alloc safe size for buffer -  i.e. max for base 2
		char_type buf[1 + 8 * sizeof(TInt)]{};
		auto [ptr, ec] = std::to_chars(buf, buf + sizeof(buf) - 1, i, r);
		if (ec != std::errc())
			return false;
		auto num_len = static_cast<size_type>(ptr - buf);
		return handle_insert(length(), buf, num_len, false);
	}
#endif

	bool set_u(unsigned int u, Radix r = base10)
	{
		clear();//Always start empty
#ifdef ARDUINO 
		char_type buf[1 + 8 * sizeof(unsigned long)]{};//alloc max size for base 2
		return handle_insert(length(), utoa(u, buf, r), false);
#else
		return set_chars(u, r);
#endif
	}
	bool set_ul(unsigned long ul, Radix r = base10)
	{
		clear();//Always start empty
#ifdef ARDUINO 
		char_type buf[1 + 8 * sizeof(unsigned long)]{};//alloc max size for base 2
		return handle_insert(length(), ultoa(ul, buf, r), false);
#else
		return set_chars(ul, r);
#endif
	}
	bool set_i(int i, Radix r = base10)
	{
		clear();//Always start empty
#ifdef ARDUINO 
		char_type buf[2 + 8 * sizeof(int)]{};//alloc max size for base 2
		return handle_insert(length(), itoa(i, buf, r), false);
#else
		return set_chars(i, r);
#endif
	}
	bool set_l(long l, Radix r = base10)
	{
		clear();//Always start empty
#ifdef ARDUINO 
		char_type buf[2 + 8 * sizeof(long)]{};//alloc max size for base 2
		return handle_insert(length(), ltoa(l, buf, r), false);
#else
		return set_chars(l, r);
#endif
	}
	bool set_f(double f, size_type decPlaces)
	{
		return set_f(f, decPlaces + 2, decPlaces);
	}

	bool set_f(double f, size_type width, size_type prec)
	{
		clear();//Always start empty
		if (prec >= width)
			return false;
		if (!valid_len(width + 2))
			return notify_overrun();
		char_type buf[1 + 4 * sizeof(double)]{}; //Worst case size
#ifdef ARDUINO
		return handle_insert(length(),
			dtostrf(f, static_cast<int8_t>(width), static_cast<uint8_t>(prec), buf), false);
#else
		//We alloc safe size for format buffer 
		char_type fmt[24]{};
		snprintf(fmt, sizeof(fmt) - 1, "%%%d.%df", width, prec);
		snprintf(buf, sizeof(buf) - 1, fmt, f);
		return handle_insert(length(), buf, false);
#endif
	}
	bool notify_overrun()
	{
		printf("Fixed string : '%s' has overrun\n", c_str());
		return false;
	}
	bool notify_overrun(const_pointer data, size_type len)
	{
		printf("Fixed string too small cannot add: '%.*s'\n", len, data);
		return false;
	}
public:
#ifdef ARDUINO
	friend FixedString operator+(const String& s, const FixedString& rhs) { return FixedString(s) += rhs; }
	friend FixedString operator+(FlashPtr s, const FixedString& rhs) { return FixedString(s) += rhs; }
#endif
	friend FixedString operator+(const_pointer s, const FixedString& rhs) { return FixedString(s) += rhs; }
	template<typename TRhs> //All others
	friend FixedString operator+(TRhs lhs, const FixedString& rhs) { return FixedString(lhs) += rhs; }
};

///////////////////////////////////////////////////////////////////////////////////////////
#endif
