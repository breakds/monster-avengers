;;;; amulet-panel.lisp

(in-package #:monster-avengers.simple-web)

(eval-when (:compile-toplevel :load-toplevel :execute)
  (enable-jsx-reader))

(def-widget amulet-skill-panel (language skill-id skill-points slot-id callback)
    ()
  #jsx(:div ((class-name "form-inline")
             (style :margin-bottom "20px"))
            (:div ((class-name "form-group")
                   (style :margin-right "20px"))
                  (:select ((class-name "form-control")
                            (value skill-id)
                            (on-change (lambda (e)
                                         (funcall callback slot-id
                                                  "skill-id" (@ e target value)))))
                           (:option ((value "-1")) (lang-text ("en" "Blank")
                                                              ("zh" "无技能")))
                           (chain skill-systems 
                                  (filter (lambda (skill id)
                                            (> id 0)))
                                  (sort (lambda (x y)
                                          (if (< (@ x name en) (@ y name en))
                                              -1 1)))
                                  (map (lambda (system id)
                                         (:option ((value (@ system id)))
                                                  (lang-text ("zh" (@ system name jp))
                                                             ("en" (@ system name en)))))))))
            (:div ((class-name "form-group"))
                  (:select ((class-name "form-control")
                            (value skill-points)
                            (on-change (lambda (e)
                                         (funcall callback slot-id
                                                  "points" (@ e target value)))))
                           (chain (array -10 -9 -8 -7 -6 -5 -4 -3 -2 -1 0 
                                         1 2 3 4 5 6 7 8 9 10 11 12 13 14
                                         15 16 17 18 19 20)
                                  (map (lambda (points)
                                         (:option ((value points))
                                                  (if (> points 0) 
                                                      (+ "+" points)
                                                      points)))))))))

(def-widget amulet-item (language amulet destructor)
    ()                    
  #jsx(:li ((class-name "list-group-item"))
           (:div ((class-name "row"))
                 (:div ((class-name "col-md-2")
                        (style :font-family "monospace"))
                       (funcall (lambda (x) (case x
                                              ("0" "---")
                                              ("1" "o--")
                                              ("2" "oo-")
                                              ("3" "ooo")))
                                (aref amulet 0)))
                 (:div ((class-name "col-md-4"))
                         (when (> (@ amulet length) 1)
                           (let ((content ""))
                             (setf content 
                                   (+ content 
                                      (lang-text ("zh" (@ (aref skill-systems
                                                                (aref amulet 1))
                                                          name jp))
                                                 ("en" (@ (aref skill-systems
                                                                (aref amulet 1))
                                                          name en)))))
                             (let ((points (aref amulet 2)))
                               (setf content (+ content (if (> points 0) 
                                                            (+ " +" points)
                                                            (+ " " points)))))
                             content)))
                 (:div ((class-name "col-md-4"))
                       (let ((content ""))
                         (when (> (@ amulet length) 3)
                           (setf content (+ content 
                                            (lang-text ("zh" (@ (aref skill-systems
                                                                      (aref amulet 3))
                                                                name jp))
                                                       ("en" (@ (aref skill-systems
                                                                      (aref amulet 3))
                                                                name en)))))
                           (let ((points (aref amulet 4)))
                             (setf content (+ content (if (> points 0) 
                                                          (+ " +" points)
                                                          (+ " " points)))))
                           content)))
                 (:div ((class-name "col-md-2"))
                       (:button ((class-name "btn btn-default btn-xs")
                                 (on-click (lambda () 
                                             (funcall destructor amulet))))
                                (:span ((class-name "glyphicon glyphicon-remove"))))))))

(def-widget amulet-panel (language callback amulets)
    ((state (holes "0")
            (skill-points-a 0)
            (skill-points-b 0)
            (skill-id-a -1)
            (skill-id-b -1))
     (add-amulet ()
                 (let ((amulet (array)))
                   (chain amulet (push (local-state holes)))
                   (when (and (not (= (local-state skill-points-a) 0))
                              (not (= (local-state skill-id-a) -1)))
                     (chain amulet (push (local-state skill-id-a)))
                     (chain amulet (push (local-state skill-points-a))))
                   (when (and (not (= (local-state skill-points-b) 0))
                              (not (= (local-state skill-id-b) -1)))
                     (chain amulet (push (local-state skill-id-b)))
                     (chain amulet (push (local-state skill-points-b))))
                   (when (or (> (@ amulet length) 1)
                             (> (aref amulet 0) 0))
                     (let ((new-amulets amulets))
                       (chain new-amulets (push amulet)))
                     (funcall callback new-amulets))))
     (stringify (amulet)
                (let ((result ""))
                  (loop for x in amulet
                     do (setf result (+ result "," x)))
                  result))
     (remove-amulet (amulet)
                    (let ((new-amulets 
                           (chain amulets
                                  (filter (lambda (e index a)
                                            (not (= (stringify e)
                                                    (stringify amulet))))))))
                      (funcall callback new-amulets))
                    nil)
     (handle-holes (holes)
                   (chain this (set-state (create holes holes)))
                   nil)
     (handle-change (slot-id type value)
                    (if (= type "points")
                        (if (= slot-id "a")
                            (chain this (set-state (create skill-points-a value)))
                            (chain this (set-state (create skill-points-b value))))
                        (if (= slot-id "a")
                            (chain this (set-state (create skill-id-a value)))
                            (chain this (set-state (create skill-id-b value)))))
                    nil))
  #jsx(:div ((class-name "panel panel-default"))
            (:div ((class-name "panel-heading"))
                  (lang-text ("en" "Amulets")
                             ("zh" "护石")))
            (:ul ((class-name "list-group"))
                 (chain amulets
                        (map (lambda (amulet)
                               (:amulet-item ((:language language)
                                              (:destructor (@ this remove-amulet))
                                              (:amulet amulet)))))))
            (:div ((class-name "panel-body"))
                  (:label () (lang-text ("en" "Skill A")
                                        ("zh" "技能 1")))
                  (:amulet-skill-panel ((:language language)
                                        (skill-id (local-state skill-id-a))
                                        (skill-points (local-state skill-points-a))
                                        (slot-id "a")
                                        (:callback (@ this handle-change))))
                  (:label () (lang-text ("en" "Skill B")
                                        ("zh" "技能 2")))
                  (:amulet-skill-panel ((:language language)
                                        (skill-id (local-state skill-id-b))
                                        (skill-points (local-state skill-points-b))
                                        (slot-id "b")
                                        (:callback (@ this handle-change))))
                  (:label () (lang-text ("en" "Slots")
                                        ("zh" "护石孔数")))
                  (:select ((class-name "form-control")
                            (value (local-state holes))
                            (style :margin-bottom "20px")
                            (on-change (lambda (e)
                                         (funcall (@ this handle-holes)
                                                  (@ e target value)))))
                           (:option ((value "0")) "---")
                           (:option ((value "1")) "O")
                           (:option ((value "2")) "OO")
                           (:option ((value "3")) "OOO"))
                  (:button ((class-name "btn btn-info")
                            (on-click (@ this add-amulet)))
                           (lang-text ("en" "Add")
                                      ("zh" "添加"))))))

(eval-when (:compile-toplevel :load-toplevel :execute)
  (disable-jsx-reader))
