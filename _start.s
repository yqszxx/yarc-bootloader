.text
	.global _start
	.global _sp
	.global main

_start:
	la sp, _sp
	call main
FOREVERLOOP:
	j FOREVERLOOP   # should never get here
