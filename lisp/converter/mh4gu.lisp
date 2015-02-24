;;;; mh4gu.lisp
;;;; The converter from sqlite db file to lisp dataset.

(in-package #:breakds.monster-avengers.mh4gu-converter)

(defparameter *dataset-folder* 
  (merge-pathnames "dataset/MH4GU/"
                   (asdf:system-source-directory 'monster-avengers)))

(defparameter *database-path*
  (merge-pathnames "dataset/MH4GU/mh4u.db"
                   (asdf:system-source-directory 'monster-avengers)))

(defun get-dataset-file (filename)
  (merge-pathnames filename
                   *dataset-folder*))

(defun update-skills (database)
  (let ((query-result (sqlite:execute-to-list 
                       database
                       (mkstr "SELECT skill_trees.jpn_name, skill_trees.name, "
                              "skills.jpn_name, skills.name, skills.description, "
                              "skills.required_skill_tree_points, skill_trees._id "
                              "FROM skills "
                              "INNER JOIN skill_trees "
                              "ON skills.skill_tree_id = skill_trees._id "
                              "ORDER BY skill_trees._id, required_skill_tree_points")))
        skill-systems)
    (loop 
       for previous-system = "" then system-en
       for (system-jp system-en skill-jp skill-en description-en points id)
       in query-result
       do (if (equal system-en previous-system)
              (push (list :obj t
                          :name `(:obj t :jp ,skill-jp :en ,skill-en)
                          :points (if (string= "troso up" (string-downcase skill-en))
                                      0
                                      points)
                          :description `(:obj t :en ,description-en))
                    (getf (car skill-systems) :skills))
              (push (list :obj t
                          :name `(:obj t :jp ,system-jp :en ,system-en)
                          :id (1- id)
                          :skills (list (list :obj t
                                              :name `(:obj t :jp ,skill-jp :en ,skill-en)
                                              :points points
                                              :description `(:obj t :en ,description-en))))
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
                       (mkstr "SELECT items.name, items.jpn_name, "
                              "decorations.num_slots, "
                              "skill_tree_id, point_value "
                              "FROM decorations "
                              "INNER JOIN items ON decorations._id = items._id "
                              "INNER JOIN item_to_skill_tree "
                              "ON item_to_skill_tree.item_id = items._id "
                              "ORDER BY items.name")))
        jewels)
    (loop 
       for previous-jewel = "" then jewel-en
       for (jewel-en jewel-jp slots skill-system-id points) in query-result
       do (if (equal jewel-en previous-jewel)
              (push (list :obj t
                          :skill-id (1- skill-system-id)
                          :points points)
                    (getf (car jewels) :effects))
              (push (list :obj t
                          :name `(:obj t :en ,jewel-en :jp ,jewel-jp)
                          :slots slots
                          :effects (list (list :obj t 
                                               :skill-id (1- skill-system-id)
                                               :points points)))
                    jewels)))
    (with-open-file (out (get-dataset-file "jewels.lisp")
                         :direction :output
                         :if-exists :supersede)
      (write (reverse jewels) :stream out))
    (format t "[ ok ] ~a jewels written to jewels.lisp~%"
            (length jewels))))

(defun update-armors (database)
  (let ((query-result (sqlite:execute-to-list
                       database
                       (mkstr "SELECT items.name, items.jpn_name, armor.slot, "
                              "armor.num_slots, "
                              "items.rarity, armor.hunter_type, armor.gender, "
                              "armor.defense, armor.max_defense, "
                              "armor.fire_res, armor.thunder_res, "
                              "armor.dragon_res, armor.water_res, armor.ice_res, "
                              "skill_tree_id, point_value "
                              "FROM armor "
                              "INNER JOIN items ON items._id = armor._id "
                              "INNER JOIN item_to_skill_tree "
                              "ON item_to_skill_tree.item_id = items._id "
                              "ORDER BY items.name")))
        armors)
    (loop 
       for previous-armor = "" then armor-en
       for (armor-en armor-jp part slots rare type gender
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
                          :name `(:obj t :en ,armor-en :jp ,armor-jp)
                          :par part
                          :gender gender
                          :type type
                          :slots slots
                          :rare rare
                          :min-defense min-defense
                          :max-defense max-defense
                          :resistence (list :obj t 
                                            :fire fire
                                            :thunder thunder
                                            :dragon dragon
                                            :water water
                                            :ice ice)
                          :effects (list (list :obj t 
                                               :skill-id (1- skill-system-id)
                                               :points points)))
                    armors)))
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
    (update-armors database)))


