(define (domain nestedqnp08)
	(:predicates (gr0_1) (gr0_2) (gr0_3) (gr0_4) (gr0_5) (gr0_6) (gr0_7) (gr0_8) (p) (q) (r))
	(:action a1
		:parameters ()
		:precondition (and (q) (gr0_1) (p))
		:effect (and  (oneof (gr0_1) (not (gr0_1))) (not (p)) (not (q))))
	(:action a2
		:parameters ()
		:precondition (and (q) (gr0_2) (p) (not (gr0_1)))
		:effect (and (gr0_1) (oneof (gr0_2) (not (gr0_2))) (not (p)) (not (q))))
	(:action a3
		:parameters ()
		:precondition (and (q) (gr0_3) (p) (not (gr0_2)))
		:effect (and (gr0_1) (gr0_2) (oneof (gr0_3) (not (gr0_3))) (not (p)) (not (q))))
	(:action a4
		:parameters ()
		:precondition (and (q) (gr0_4) (p) (not (gr0_3)))
		:effect (and (gr0_1) (gr0_2) (gr0_3) (oneof (gr0_4) (not (gr0_4))) (not (p)) (not (q))))
	(:action a5
		:parameters ()
		:precondition (and (q) (gr0_5) (p) (not (gr0_4)))
		:effect (and (gr0_1) (gr0_2) (gr0_3) (gr0_4) (oneof (gr0_5) (not (gr0_5))) (not (p)) (not (q))))
	(:action a6
		:parameters ()
		:precondition (and (q) (gr0_6) (p) (not (gr0_5)))
		:effect (and (gr0_1) (gr0_2) (gr0_3) (gr0_4) (gr0_5) (oneof (gr0_6) (not (gr0_6))) (not (p)) (not (q))))
	(:action a7
		:parameters ()
		:precondition (and (q) (gr0_7) (p) (not (gr0_6)))
		:effect (and (gr0_1) (gr0_2) (gr0_3) (gr0_4) (gr0_5) (gr0_6) (oneof (gr0_7) (not (gr0_7))) (not (p)) (not (q))))
	(:action a8
		:parameters ()
		:precondition (and (q) (gr0_8) (p) (not (gr0_7)))
		:effect (and (gr0_1) (gr0_2) (gr0_3) (gr0_4) (gr0_5) (gr0_6) (gr0_7) (oneof (gr0_8) (not (gr0_8))) (not (p)) (not (q))))
	(:action b
		:parameters ()
		:precondition (not (p))
		:effect (oneof (p) (not (p))))
	(:action c
		:parameters ()
		:precondition (not (q))
		:effect (and (oneof (q) (not (q))) (r)))
	(:action d
		:parameters ()
		:precondition (and (r))
		:effect (and (q) (not (r)))))
