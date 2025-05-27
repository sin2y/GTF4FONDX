(define (problem BlocksColumns-6-3-9) ;spec: 010011
  (:domain BlocksColumns)
  (:objects
    c0 c1 c2 c3 c4 c5 - column
    p0 p1 p2 - person
    g0 - gripper
  )
  (:init
    (in p0 c0)
    (in p1 c2)
    (in p2 c3)
    (blocks-gr0 c1)
    (blocks-gr0 c4)
    (blocks-gr0 c5)
    (in g0 c0)
    (empty g0)
    (adjacent c0 c1)
    (adjacent c1 c2)
    (adjacent c2 c3)
    (adjacent c3 c4)
    (adjacent c4 c5)
    (adjacent c1 c0)
    (adjacent c2 c1)
    (adjacent c3 c2)
    (adjacent c4 c3)
    (adjacent c5 c4)
  )
  (:goal
    (and
      (in p0 c5)
      (in p1 c5)
      (in p2 c5)
    )
  )
  (:fairness :a (pick g0 c0) :b (drop g0 c0))
  (:fairness :a (pick g0 c1) :b (drop g0 c1))
  (:fairness :a (pick g0 c2) :b (drop g0 c2))
  (:fairness :a (pick g0 c3) :b (drop g0 c3))
  (:fairness :a (pick g0 c4) :b (drop g0 c4))
  (:fairness :a (pick g0 c5) :b (drop g0 c5))
)