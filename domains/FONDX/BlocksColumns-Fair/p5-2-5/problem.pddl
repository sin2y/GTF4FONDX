(define (problem BlocksColumns-Fair-5-2-5) ;spec: 01011
  (:domain BlocksColumns-Fair)
  (:objects
    c0 c1 c2 c3 c4 - column
    p0 p1 - person
    g0 - gripper
  )
  (:init
    (in p0 c0)
    (in p1 c2)
    (blocks-gr0 c1)
    (blocks-gr0 c3)
    (blocks-gr0 c4)
    (in g0 c0)
    (empty g0)
    (LR-adjacent LEFT c0)
    (LR-adjacent c0 c1)
    (LR-adjacent c1 c2)
    (LR-adjacent c2 c3)
    (LR-adjacent c3 c4)
    (LR-adjacent c4 RIGHT)
  )
  (:goal
    (and
      (in p0 c4)
      (in p1 c4)
    )
  )
  (:fairness :a (pick g0 c0) :b (drop g0 c0))
  (:fairness :a (pick g0 c1) :b (drop g0 c1))
  (:fairness :a (pick g0 c2) :b (drop g0 c2))
  (:fairness :a (pick g0 c3) :b (drop g0 c3))
  (:fairness :a (pick g0 c4) :b (drop g0 c4))
  (:fairness :a (move-person p0 c0 c1 c2) (move-person p0 c1 c2 c3) (move-person p0 c2 c3 c4) (move-person p1 c0 c1 c2) (move-person p1 c1 c2 c3) (move-person p1 c2 c3 c4))
)