(define (domain HallsFloors)
  (:requirements :typing :strips :non-deterministic :conditional-effects)
  (:types floor corner - object)
  (:predicates 
    (on ?f - floor)
    (visited-corner ?c - corner ?f - floor)
    (disl-gr0)
    (disr-gr0)
    (dist-gr0)
    (disb-gr0)
    (visited-floor ?f - floor)
    (connected ?f1 ?f2 - floor)
    (stair-in-corner ?f1 - floor ?f2 - floor ?corner - corner)
  )
  
  (:constants tl tr bl br - corner)

      (:action mvr
       :parameters  ()
       :precondition (and (disr-gr0) (or (not (dist-gr0)) (not (disb-gr0))))
       :effect (and (oneof (disr-gr0)(not (disr-gr0))) (disl-gr0))
       )
       
      (:action mvl
       :parameters  ()
       :precondition (and (disl-gr0) (or (not (dist-gr0)) (not (disb-gr0))))
       :effect (and (oneof (disl-gr0)(not (disl-gr0))) (disr-gr0))
       )
       
      (:action mvu
       :parameters  ()
       :precondition (and (dist-gr0) (or (not (disl-gr0)) (not (disr-gr0))))
       :effect (and (oneof (dist-gr0)(not (dist-gr0))) (disb-gr0))
       )
       
      (:action mvd
       :parameters  ()
       :precondition (and (disb-gr0) (or (not (disl-gr0)) (not (disr-gr0))))
       :effect (and (oneof (disb-gr0)(not (disb-gr0))) (dist-gr0))
       )
       
       (:action mark-corner
         :parameters (?c - corner ?f - floor)
         :precondition (and 
           (on ?f)
           (or 
                (and (= ?c br) (not (disr-gr0)) (not (disb-gr0)))
                (and (= ?c bl) (not (disl-gr0)) (not (disb-gr0)))
                (and (= ?c tl) (not (disl-gr0)) (not (dist-gr0)))
                (and (= ?c tr) (not (disr-gr0)) (not (dist-gr0)))
           )
         )
         :effect (and
                (visited-corner ?c ?f)
         ))

       (:action mark-floor
       :parameters  (?f - floor)
       :precondition (and (on ?f) (visited-corner tl ?f) (visited-corner tr ?f) (visited-corner bl ?f) (visited-corner br ?f))
       :effect (and (visited-floor ?f)))
                
       (:action goto
       :parameters  (?f1 ?f2 - floor)
       :precondition (and (on ?f1) (connected ?f1 ?f2)
                                (or 
                                        (and (stair-in-corner ?f1 ?f2 tl) (not (disl-gr0)) (not (dist-gr0)))
                                        (and (stair-in-corner ?f1 ?f2 tr) (not (disr-gr0)) (not (dist-gr0)))
                                        (and (stair-in-corner ?f1 ?f2 bl) (not (disl-gr0)) (not (disb-gr0)))
                                        (and (stair-in-corner ?f1 ?f2 br) (not (disr-gr0)) (not (disb-gr0)))
                                )
                                )
       :effect (and (on ?f2) (not (on ?f1))
       ))
)
