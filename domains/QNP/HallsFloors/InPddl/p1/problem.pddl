(define (problem HallsFloors-1) ;spec: [[0]]
  (:domain HallsFloors)
  (:objects
    floor1 - floor
  )
  (:init
    (on floor1)
    (disb-gr0)
    (disr-gr0)
  )
  (:goal
    (and
      (visited-floor floor1)
    )
  )
  (:fairness :a (mvr) :b (mvl))
  (:fairness :a (mvl) :b (mvr))
  (:fairness :a (mvd) :b (mvu))
  (:fairness :a (mvu) :b (mvd))
)