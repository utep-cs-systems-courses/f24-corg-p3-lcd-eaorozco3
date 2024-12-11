	.text
	.balign 2
	.global increment_speed
	.extern alienSpeed
increment_speed:
	mov.b #4, r12
	cmp.b r12, &alienSpeed
	jc out
	add.b #1, &alienSpeed
out:
	pop r0
	
