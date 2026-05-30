(define (domain nestedqnp04)
	(:predicates (gr0_1) (gr0_2) (gr0_3) (gr0_4) (p) (q) (r))
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
