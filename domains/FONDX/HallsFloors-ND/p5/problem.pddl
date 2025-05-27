(define (problem HallsFloors-ND-5) ;spec: [[0, 2, 4, 6, 0], [2], [3], [4], [5]]
  (:domain HallsFloors-ND)
  (:objects
    floor1 floor2 floor3 floor4 floor5 - floor
  )
  (:init
    (on floor1)
    (disb-gr0)
    (disr-gr0)
    (connected floor1 floor2)
    (connected floor2 floor1)
    (connected floor2 floor3)
    (connected floor3 floor2)
    (connected floor3 floor4)
    (connected floor4 floor3)
    (connected floor4 floor5)
    (connected floor5 floor4)
  )
  (:goal
    (and
      (visited-floor floor1)
      (visited-floor floor2)
      (visited-floor floor3)
      (visited-floor floor4)
      (visited-floor floor5)
    )
  )
  (:fairness :a (mvr) :b (mvl))
  (:fairness :a (mvl) :b (mvr))
  (:fairness :a (mvd) :b (mvu))
  (:fairness :a (mvu) :b (mvd))
)