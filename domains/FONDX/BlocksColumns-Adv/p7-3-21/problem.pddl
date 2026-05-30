(define (problem BlocksColumns-Adv-7-3-21) ;spec: 0101011
  (:domain BlocksColumns-Adv)
  (:objects
    c0 c1 c2 c3 c4 c5 c6 - column
    p0 p1 p2 - person
    g0 - gripper
  )
  (:init
    (in p0 c0)
    (in p1 c2)
    (in p2 c4)
    (blocks-gr0 c1)
    (blocks-gr0 c3)
    (blocks-gr0 c5)
    (blocks-gr0 c6)
    (in g0 c0)
    (empty g0)
    (LR-adjacent LEFT c0)
    (LR-adjacent c0 c1)
    (LR-adjacent c1 c2)
    (LR-adjacent c2 c3)
    (LR-adjacent c3 c4)
    (LR-adjacent c4 c5)
    (LR-adjacent c5 c6)
    (LR-adjacent c6 RIGHT)
  )
  (:goal
    (and
      (in p0 c6)
      (in p1 c6)
      (in p2 c6)
    )
  )
  (:fairness :a (pick g0 c0) :b (drop g0 c0))
  (:fairness :a (pick g0 c1) :b (drop g0 c1))
  (:fairness :a (pick g0 c2) :b (drop g0 c2))
  (:fairness :a (pick g0 c3) :b (drop g0 c3))
  (:fairness :a (pick g0 c4) :b (drop g0 c4))
  (:fairness :a (pick g0 c5) :b (drop g0 c5))
  (:fairness :a (pick g0 c6) :b (drop g0 c6))
)