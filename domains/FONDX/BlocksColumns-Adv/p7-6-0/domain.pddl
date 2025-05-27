(define (domain BlocksColumns-Adv)
  (:requirements :typing :strips :non-deterministic)
  (:types column movable - object
          person gripper - movable)
  (:predicates 
    (in ?m - movable ?c - column)
    (blocks-gr0 ?c - column)
    (empty ?g - gripper)
    (LR-adjacent ?c1 ?c2 - column)
  )
  
  (:constants LEFT RIGHT - column)

       (:action move-person
       :parameters  (?p - person ?c0 ?c1 ?c2 - column)
       :precondition (and (in ?p ?c1) (LR-adjacent ?c0 ?c1) (LR-adjacent ?c1 ?c2))
       :effect (and
       (when
            (and (or (= ?c0 LEFT) (blocks-gr0 ?c0)) 
                 (not (blocks-gr0 ?c2)) (not (= ?c2 RIGHT)))
            (and (not (in ?p ?c1)) (in ?p ?c2))
       )
       (when
            (and (or (= ?c2 RIGHT) (blocks-gr0 ?c2)) 
                 (not (blocks-gr0 ?c0)) (not (= ?c0 LEFT)))
            (and (not (in ?p ?c1)) (in ?p ?c0))
       )
       (when 
            (and (not (= ?c0 LEFT)) (not (= ?c2 RIGHT))
                 (not (blocks-gr0 ?c0)) (not (blocks-gr0 ?c2)))
            (and (not (in ?p ?c1)) (oneof (in ?p ?c0) (in ?p ?c2)))
       )
       ))
       
       (:action move-gripper
       :parameters  (?g - gripper ?c1 ?c2 - column)
       :precondition (and (in ?g ?c1) (or (LR-adjacent ?c1 ?c2) (LR-adjacent ?c2 ?c1)) (not (= ?c2 LEFT)) (not (= ?c2 RIGHT)))
       :effect (and (not (in ?g ?c1)) (in ?g ?c2)))
       
       (:action pick
       :parameters  (?g - gripper ?c - column)
       :precondition (and (in ?g ?c) (blocks-gr0 ?c) (empty ?g))
       :effect (and (oneof (blocks-gr0 ?c)(not (blocks-gr0 ?c))) (not (empty ?g))))
       
       (:action drop
       :parameters  (?g - gripper ?c - column)
       :precondition (and 
                         (forall (?p - person) (not (in ?p ?c)))
                         (in ?g ?c) (not (empty ?g)))
       :effect (and (blocks-gr0 ?c) (empty ?g)))
)
