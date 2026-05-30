(define (problem BlocksColumns-3-1-1) ;spec: 011
  (:domain BlocksColumns)
  (:objects
    c0 c1 c2 - column
    p0 - person
    g0 - gripper
  )
  (:init
    (in p0 c0)
    (blocks-gr0 c1)
    (blocks-gr0 c2)
    (in g0 c0)
    (empty g0)
    (adjacent c0 c1)
    (adjacent c1 c2)
    (adjacent c1 c0)
    (adjacent c2 c1)
  )
  (:goal
    (and
      (in p0 c2)
    )
  )
  (:fairness :a (pick g0 c0) :b (drop g0 c0))
  (:fairness :a (pick g0 c1) :b (drop g0 c1))
  (:fairness :a (pick g0 c2) :b (drop g0 c2))
)