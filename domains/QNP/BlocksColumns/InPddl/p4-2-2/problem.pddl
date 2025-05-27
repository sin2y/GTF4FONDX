(define (problem BlocksColumns-4-2-2) ;spec: 0101
  (:domain BlocksColumns)
  (:objects
    c0 c1 c2 c3 - column
    p0 p1 - person
    g0 - gripper
  )
  (:init
    (in p0 c0)
    (in p1 c2)
    (blocks-gr0 c1)
    (blocks-gr0 c3)
    (in g0 c0)
    (empty g0)
    (adjacent c0 c1)
    (adjacent c1 c2)
    (adjacent c2 c3)
    (adjacent c1 c0)
    (adjacent c2 c1)
    (adjacent c3 c2)
  )
  (:goal
    (and
      (in p0 c3)
      (in p1 c3)
    )
  )
  (:fairness :a (pick g0 c0) :b (drop g0 c0))
  (:fairness :a (pick g0 c1) :b (drop g0 c1))
  (:fairness :a (pick g0 c2) :b (drop g0 c2))
  (:fairness :a (pick g0 c3) :b (drop g0 c3))
)