/* SPDX-License-Identifier: LGPL-2.1-only */
/*
 * Copyright (c) 2013 Thomas Graf <tgraf@suug.ch>
 */

#include "nl-default.h"

#include <check.h>
#include <netlink/addr.h>
#include <netlink/route/addr.h>

#include "cksuite-all.h"

START_TEST(addr_alloc)
{
	struct nl_addr *addr;

	addr = nl_addr_alloc(16);
	ck_assert_msg(addr, "Allocation should not return NULL");

	ck_assert_msg(nl_addr_iszero(addr) != 0,
		      "New empty address should be all zeros");

	ck_assert_msg(nl_addr_get_family(addr) == AF_UNSPEC,
		      "New empty address should have family AF_UNSPEC");

	ck_assert_msg(nl_addr_get_prefixlen(addr) == 0,
		      "New empty address should have prefix length 0");

	ck_assert_msg(!nl_addr_shared(addr),
		      "New empty address should not be shared");

	ck_assert_msg(nl_addr_get(addr) == addr,
		      "nl_addr_get() should return pointer to address");

	ck_assert_msg(nl_addr_shared(addr) != 0,
		      "Address should be shared after call to nl_addr_get()");

	nl_addr_put(addr);

	ck_assert_msg(
		!nl_addr_shared(addr),
		"Address should not be shared after call to nl_addr_put()");

	ck_assert_msg(nl_addr_fill_sockaddr(addr, NULL, 0) != 0,
		      "Socket address filling should fail for empty address");

	nl_addr_put(addr);
}
END_TEST

START_TEST(addr_binary_addr)
{
	struct nl_addr *addr, *addr2;
	char baddr[4] = { 0x1, 0x2, 0x3, 0x4 };
	char baddr2[6] = { 0x1, 0x2, 0x3, 0x4, 0x5, 0x6 };

	addr = nl_addr_alloc(4);
	ck_assert_msg(addr != NULL, "Allocation should not return NULL");

	ck_assert_msg(nl_addr_set_binary_addr(addr, baddr, 4) >= 0,
		      "Valid binary address should be settable");

	ck_assert_msg(
		nl_addr_get_prefixlen(addr) == 0,
		"Prefix length should be unchanged after nl_addr_set_binary_addr()");

	ck_assert_msg(nl_addr_get_len(addr) == 4, "Address length should be 4");

	ck_assert_msg(
		nl_addr_set_binary_addr(addr, baddr2, 6) != 0,
		"Should not be able to set binary address exceeding maximum length");

	ck_assert_msg(nl_addr_get_len(addr) == 4,
		      "Address length should still be 4");

	ck_assert_msg(
		nl_addr_guess_family(addr) == AF_INET,
		"Binary address of length 4 should be guessed as AF_INET");

	ck_assert_msg(memcmp(baddr, nl_addr_get_binary_addr(addr), 4) == 0,
		      "Binary address mismatches");

	addr2 = nl_addr_build(AF_UNSPEC, baddr, 4);
	ck_assert_msg(addr2 != NULL, "Building of address should not fail");

	nl_addr_set_prefixlen(addr, 32);
	ck_assert_msg(
		nl_addr_get_prefixlen(addr) == 32,
		"Prefix length should be successful changed after nl_addr_set_prefixlen()");

	ck_assert_msg(!nl_addr_cmp(addr, addr2),
		      "Addresses built from same binary address should match");

	nl_addr_put(addr);
	nl_addr_put(addr2);
}
END_TEST

START_TEST(addr_parse4)
{
	struct nl_addr *addr4, *clone;
	struct sockaddr_in sin;
	socklen_t len = sizeof(sin);
	char *addr_str = "10.0.0.1/16";
	char buf[128];

	ck_assert_msg(nl_addr_parse(addr_str, AF_INET6, &addr4) != 0,
		      "Should not be able to parse IPv4 address in IPv6 mode");

	ck_assert_msg(nl_addr_parse(addr_str, AF_UNSPEC, &addr4) == 0,
		      "Should be able to parse \"%s\"", addr_str);

	ck_assert_msg(nl_addr_get_family(addr4) == AF_INET,
		      "Address family should be AF_INET");

	ck_assert_msg(nl_addr_get_prefixlen(addr4) == 16,
		      "Prefix length should be 16");

	ck_assert_msg(!nl_addr_iszero(addr4),
		      "Address should not be all zeroes");

	clone = nl_addr_clone(addr4);
	ck_assert_msg(clone != NULL, "Cloned address should not be NULL");

	ck_assert_msg(nl_addr_cmp(addr4, clone) == 0,
		      "Cloned address should not mismatch original");

	ck_assert_msg(nl_addr_fill_sockaddr(addr4, (struct sockaddr *)&sin,
					    &len) == 0,
		      "Should be able to fill socketaddr");

	ck_assert_msg(
		!strcmp(nl_addr2str(addr4, buf, sizeof(buf)), addr_str),
		"Address translated back to string does not match original");

	nl_addr_put(addr4);
	nl_addr_put(clone);
}
END_TEST

START_TEST(addr_parse6)
{
	struct nl_addr *addr6, *clone;
	struct sockaddr_in6 sin;
	socklen_t len = sizeof(sin);
	char *addr_str = "2001:1:2::3/64";
	char buf[128];

	ck_assert_msg(nl_addr_parse(addr_str, AF_INET, &addr6) != 0,
		      "Should not be able to parse IPv6 address in IPv4 mode");

	ck_assert_msg(nl_addr_parse(addr_str, AF_UNSPEC, &addr6) == 0,
		      "Should be able to parse \"%s\"", addr_str);

	ck_assert_msg(nl_addr_get_family(addr6) == AF_INET6,
		      "Address family should be AF_INET6");

	ck_assert_msg(nl_addr_get_prefixlen(addr6) == 64,
		      "Prefix length should be 64");

	ck_assert_msg(!nl_addr_iszero(addr6),
		      "Address should not be all zeroes");

	clone = nl_addr_clone(addr6);
	ck_assert_msg(clone != NULL, "Cloned address should not be NULL");

	ck_assert_msg(nl_addr_cmp(addr6, clone) == 0,
		      "Cloned address should not mismatch original");

	ck_assert_msg(nl_addr_fill_sockaddr(addr6, (struct sockaddr *)&sin,
					    &len) == 0,
		      "Should be able to fill socketaddr");

	ck_assert_msg(
		!strcmp(nl_addr2str(addr6, buf, sizeof(buf)), addr_str),
		"Address translated back to string does not match original");

	nl_addr_put(addr6);
	nl_addr_put(clone);
}
END_TEST

START_TEST(addr_info)
{
	struct nl_addr *addr;
	char *addr_str = "127.0.0.1";
	struct addrinfo *result;

	ck_assert_msg(nl_addr_parse(addr_str, AF_UNSPEC, &addr) == 0,
		      "Parsing of valid address should not fail");

	ck_assert_msg(nl_addr_info(addr, &result) == 0,
		      "getaddrinfo() on loopback address should work");

	freeaddrinfo(result);
	nl_addr_put(addr);
}
END_TEST

START_TEST(addr_flags2str)
{
	int ifa_flags = IFA_F_TENTATIVE | IFA_F_DADFAILED;
	int ifa_flags2;
	char buf[128];

	rtnl_addr_flags2str(ifa_flags, buf, sizeof(buf));
	ck_assert_str_eq(buf, "dadfailed,tentative");

	ifa_flags2 = rtnl_addr_str2flags(buf);
	ck_assert_int_eq(ifa_flags2, ifa_flags);
}
END_TEST

Suite *make_nl_addr_suite(void)
{
	Suite *suite = suite_create("Abstract addresses");
	TCase *tc = tcase_create("Core");

	tcase_add_test(tc, addr_alloc);
	tcase_add_test(tc, addr_binary_addr);
	tcase_add_test(tc, addr_parse4);
	tcase_add_test(tc, addr_parse6);
	tcase_add_test(tc, addr_info);
	tcase_add_test(tc, addr_flags2str);
	suite_add_tcase(suite, tc);

	return suite;
}
