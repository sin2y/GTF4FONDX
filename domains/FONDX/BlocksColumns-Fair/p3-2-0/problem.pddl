(define (problem BlocksColumns-Fair-3-2-0) ;spec: 001
  (:domain BlocksColumns-Fair)
  (:objects
    c0 c1 c2 - column
    p0 p1 - person
    g0 - gripper
  )
  (:init
    (in p0 c0)
    (in p1 c1)
    (blocks-gr0 c2)
    (in g0 c0)
    (empty g0)
    (LR-adjacent LEFT c0)
    (LR-adjacent c0 c1)
    (LR-adjacent c1 c2)
    (LR-adjacent c2 RIGHT)
  )
  (:goal
    (and
      (in p0 c2)
      (in p1 c2)
    )
  )
  (:fairness :a (pick g0 c0) :b (drop g0 c0))
  (:fairness :a (pick g0 c1) :b (drop g0 c1))
  (:fairness :a (pick g0 c2) :b (drop g0 c2))
  (:fairness :a (move-person p0 c0 c1 c2) (move-person p1 c0 c1 c2))
)