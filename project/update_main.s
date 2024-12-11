	.text
	.global update_transitions
update_transitions:
	CALL #increment_score
	CALL #increment_speed
	pop r0
