(define (problem HallsFloors-ND-2) ;spec: [[0, 2], [2]]
  (:domain HallsFloors-ND)
  (:objects
    floor1 floor2 - floor
  )
  (:init
    (on floor1)
    (disb-gr0)
    (disr-gr0)
    (connected floor1 floor2)
    (connected floor2 floor1)
  )
  (:goal
    (and
      (visited-floor floor1)
      (visited-floor floor2)
    )
  )
  (:fairness :a (mvr) :b (mvl))
  (:fairness :a (mvl) :b (mvr))
  (:fairness :a (mvd) :b (mvu))
  (:fairness :a (mvu) :b (mvd))
)