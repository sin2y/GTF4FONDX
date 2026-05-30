(define (problem HallsFloors-3-2) ;spec: [[0, 2, 4], [2, 3], []]
  (:domain HallsFloors)
  (:objects
    floor1 floor2 floor3 - floor
  )
  (:init
    (on floor1)
    (disb-gr0)
    (disr-gr0)
    (connected floor1 floor2)
    (connected floor2 floor1)
    (connected floor1 floor3)
    (connected floor3 floor1)
    (stair-in-corner floor1 floor2 tr)
    (stair-in-corner floor2 floor1 tr)
    (stair-in-corner floor1 floor3 br)
    (stair-in-corner floor3 floor1 br)
  )
  (:goal
    (and
      (visited-floor floor1)
      (visited-floor floor2)
      (visited-floor floor3)
    )
  )
  (:fairness :a (mvr) :b (mvl))
  (:fairness :a (mvl) :b (mvr))
  (:fairness :a (mvd) :b (mvu))
  (:fairness :a (mvu) :b (mvd))
)