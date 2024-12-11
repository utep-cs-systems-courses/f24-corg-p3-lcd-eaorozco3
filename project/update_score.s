	.text
	.balign 2
	.global increment_score
	.extern scoreValue
increment_score:
	add #1, &scoreValue
	pop r0
