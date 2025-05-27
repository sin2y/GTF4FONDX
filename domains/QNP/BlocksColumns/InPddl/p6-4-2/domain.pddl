(define (domain BlocksColumns)
  (:requirements :typing :strips :non-deterministic)
  (:types column movable - object
          person gripper - movable)
  (:predicates 
    (in ?m - movable ?c - column)
    (blocks-gr0 ?c - column)
    (empty ?g - gripper)
    (adjacent ?c1 ?c2 - column)
  )

  (:action move-person
   :parameters  (?p - person ?c1 ?c2 - column)
   :precondition (and (in ?p ?c1) (adjacent ?c1 ?c2) (not (blocks-gr0 ?c2)))
   :effect (and (not (in ?p ?c1)) (in ?p ?c2))
  )

  (:action move-gripper
   :parameters  (?g - gripper ?c1 ?c2 - column)
   :precondition (and (in ?g ?c1) (adjacent ?c1 ?c2))
   :effect (and (not (in ?g ?c1)) (in ?g ?c2))
  )

  (:action pick
   :parameters  (?g - gripper ?c - column)
   :precondition (and (in ?g ?c) (blocks-gr0 ?c) (empty ?g))
   :effect (and (oneof (blocks-gr0 ?c)(not (blocks-gr0 ?c))) (not (empty ?g)))
  )

  (:action drop
   :parameters  (?g - gripper ?c - column)
   :precondition (and 
                      (forall (?p - person) (not (in ?p ?c)))
                      (in ?g ?c) (not (empty ?g)))
   :effect (and (blocks-gr0 ?c) (empty ?g))
  )
)

