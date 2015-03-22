;;;; ping_dex.lisp
;;;; The adapter from pint's dex db to lisp dataset

(in-package #:breakds.monster-avengers.mh4g-dex-adapter)

(defparameter *dataset-folder* 
  (merge-pathnames "dataset/MH4GDEX/"
                   (asdf:system-source-directory 'monster-avengers)))

(defparameter *database-path*
  (merge-pathnames "dataset/MH4GDEX/mh4gdex.sqlite"
                   (asdf:system-source-directory 'monster-avengers)))

(defun get-dataset-file (filename)
  (merge-pathnames filename
                   *dataset-folder*))

(defun update-skills (database)
  (let ((query-result (sqlite:execute-to-list
                       database
                       (mkstr "SELECT ID_SklTree_Name.SklTree_ID, SklTree_Name_0, "
                              "SklTree_Name_3, Skl_Name_0, Skl_Name_3, Pt "
                              "FROM ID_SklTree_Name "
                              "INNER JOIN DB_Skl ON "
                              "ID_SklTree_Name.SklTree_ID = DB_Skl.SklTree_ID "
                              "INNER JOIN ID_Skl_Name ON "
                              "ID_Skl_Name.Skl_ID = DB_Skl.Skl_ID ")))
        skill-systems)
    (loop 
       for previous-system = "" then system-en
       for (id system-en system-jp skill-en skill-jp points)
       in query-result
       do (if (equal system-en previous-system)
              (push (list :obj t
                          :name `(:obj t :jp ,skill-jp :en ,skill-en)
                          :points (if (string= "troso up" (string-downcase skill-en))
                                      0
                                      points))
                    (getf (car skill-systems) :skills))
              (push (list :obj t
                          :name `(:obj t :jp ,system-jp :en ,system-en)
                          :id (1- id)
                          :skills (list (list :obj t
                                              :name `(:obj t :jp ,skill-jp :en ,skill-en)
                                              :points points)))
                    skill-systems)))
    (with-open-file (out (get-dataset-file "skills.lisp")
                         :direction :output
                         :if-exists :supersede)
      (write (reverse skill-systems) :stream out))
    (format t "[ ok ] ~a skills written to skills.lisp~%"
            (length skill-systems))))

(defun update-jewels (database)
  (let ((query-result (sqlite:execute-to-list
                       database
                       (mkstr "SELECT Jew_ID, DB_Jew.Itm_ID, "
                              "       Itm_Name_0, Itm_Name_3, Slot, "
                              "       SklTree1_ID, SklTree1_Pt, "
                              "       SklTree2_ID, SklTree2_Pt "
                              "FROM DB_Jew "
                              "INNER JOIN ID_Itm_Name ON "
                              "ID_Itm_Name.Itm_ID = DB_Jew.Itm_ID "
                              "WHERE DB_Jew.Itm_ID < 1774 "
                              "GROUP BY DB_Jew.Itm_ID "
                              "ORDER BY Itm_Name_0")))
        jewels)
    (loop 
       for (external-id item-id jewel-en jewel-jp slots a-id a-points b-id b-points) in query-result
       do (push (list :obj t
                      :external-id external-id
                      :name `(:obj t :en ,jewel-en :jp ,jewel-jp)
                      :slots slots
                      :effects (append (when (> a-id -1)
                                         (list (list :obj t
                                                     :skill-id (1- a-id)
                                                     :points a-points)))
                                       (when (> b-id -1)
                                         (list (list :obj t
                                                     :skill-id (1- b-id)
                                                     :points b-points)))))
                jewels))
    (with-open-file (out (get-dataset-file "jewels.lisp")
                         :direction :output
                         :if-exists :supersede)
      (write (reverse jewels) :stream out))
    (format t "[ ok ] ~a jewels written to jewels.lisp~%"
            (length jewels))))

(defun update-items (database)
  (let* ((query-result (sqlite:execute-to-list 
                        database
                        (mkstr "SELECT Itm_ID, Itm_Name_0, Itm_Name_3 "
                               "FROM ID_Itm_Name ORDER BY Itm_ID")))
         (items (loop for (id item-en item-jp) in query-result
                   when (> id -1)
                   collect (list :obj t
                                 :name `(:obj t :en ,item-en :jp ,item-jp)
                                 :id (1- id)))))
    (with-open-file (out (get-dataset-file "items.lisp")
                         :direction :output
                         :if-exists :supersede)
      (write items :stream out))
    (format t "[ ok ] ~a items written to items.lisp~%"
            (length items))))

(defun update-armors (database)
  (let ((query-result (sqlite:execute-to-list
                       database
                       (mkstr "SELECT DB_Amr.Amr_ID, ID_Amr_Name.Amr_Name_0, "
                              "ID_Amr_Name.Amr_Name_3, Part, Slot, Rare, "
                              "BorG, MorF, Def, MaxDef, Res_Fire, "
                              "Res_Thunder, Res_Dragon, Res_Water, "
                              "Res_Ice, SklTree_ID, Pt "
                              "FROM DB_Amr "
                              "INNER JOIN ID_Amr_Name ON "
                              "ID_Amr_Name.Amr_ID = DB_Amr.Amr_ID "
                              "INNER JOIN DB_SklTreetoAmr ON "
                              "DB_SklTreetoAmr.Amr_ID = DB_Amr.Amr_ID ")))
        (material-result (sqlite:execute-to-list
                          database
                          (mkstr "SELECT Amr_ID, Itm_ID "
                                 "FROM DB_ItmtoAmr "
                                 "ORDER BY Amr_ID")))
        (materials (make-hash-table))
        armors)
    (loop for row in material-result
       do (push (second row) 
                (gethash (car row) materials nil)))
    (loop 
       for previous-armor = "" then armor-en
       for (armor-id armor-en armor-jp part slots rare type gender
                     min-defense max-defense
                     fire thunder dragon water ice
                     skill-system-id points)
       in query-result
       do (if (equal armor-en previous-armor)
              (push (list :obj t
                          :skill-id (1- skill-system-id)
                          :points points)
                    (getf (car armors) :effects))
              (push (list :obj t
                          :external-id armor-id
                          :name `(:obj t :en ,armor-en :jp ,armor-jp)
                          :part (case part
                                  (1 "Head")
                                  (2 "Body")
                                  (3 "Arms")
                                  (4 "Waist")
                                  (5 "Legs"))
                          :gender (case gender
                                    (1 "Male")
                                    (2 "Female")
                                    (0 "Both"))
                          :type (case type
                                  (1 "Blade")
                                  (2 "Gunner")
                                  (0 "BOTH"))
                          :slots slots
                          :rare rare
                          :min-defense min-defense
                          :max-defense 120 ;; TODO(breakds) fix when have data
                          :resistence (list :obj t 
                                            :fire fire
                                            :thunder thunder
                                            :dragon dragon
                                            :water water
                                            :ice ice)
                          :material (gethash armor-id materials nil)
                          :effects (list (list :obj t 
                                               :skill-id (1- skill-system-id)
                                               :points points)))
                    armors)))
    (loop for slots below 4
       do (push (list :obj t
                      :name (case slots
                              (0 '(:obj t :en "No Slot Weapon" :jp "无孔武器"))
                              (1 '(:obj t :en "Single Slot Weapon" :jp "单孔武器"))
                              (2 '(:obj t :en "Dual Slot Weapon" :jp "双孔武器"))
                              (3 '(:obj t :en "Triple Slot Weapon" :jp "三孔武器")))
                      :external-id (+ 10000 slots)
                      :part "gear"
                      :gender "BOTH"
                      :type "BOTH"
                      :slots slots
                      :rare 10
                      :min-defense 0
                      :max-defense 0
                      :resistence (list :obj t 
                                        :fire 0
                                        :thunder 0
                                        :dragon 0
                                        :water 0
                                        :ice 0)
                      :material nil
                      :effects nil)
                armors))
    (with-open-file (out (get-dataset-file "armors.lisp")
                         :direction :output
                         :if-exists :supersede)
      (write (reverse armors) :stream out))
    (format t "[ ok ] ~a armors written to armors.lisp~%"
            (length armors))))

(defun update-dataset ()
  (sqlite:with-open-database (database *database-path*)
    (update-skills database)
    (update-jewels database)
    (update-items database)
    (update-armors database)))


