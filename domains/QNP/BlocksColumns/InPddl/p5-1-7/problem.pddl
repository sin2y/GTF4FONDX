(define (problem BlocksColumns-5-1-7) ;spec: 01111
  (:domain BlocksColumns)
  (:objects
    c0 c1 c2 c3 c4 - column
    p0 - person
    g0 - gripper
  )
  (:init
    (in p0 c0)
    (blocks-gr0 c1)
    (blocks-gr0 c2)
    (blocks-gr0 c3)
    (blocks-gr0 c4)
    (in g0 c0)
    (empty g0)
    (adjacent c0 c1)
    (adjacent c1 c2)
    (adjacent c2 c3)
    (adjacent c3 c4)
    (adjacent c1 c0)
    (adjacent c2 c1)
    (adjacent c3 c2)
    (adjacent c4 c3)
  )
  (:goal
    (and
      (in p0 c4)
    )
  )
  (:fairness :a (pick g0 c0) :b (drop g0 c0))
  (:fairness :a (pick g0 c1) :b (drop g0 c1))
  (:fairness :a (pick g0 c2) :b (drop g0 c2))
  (:fairness :a (pick g0 c3) :b (drop g0 c3))
  (:fairness :a (pick g0 c4) :b (drop g0 c4))
)