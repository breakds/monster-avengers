;;;; backend.lisp

(in-package #:monster-avengers.simple-web)

;;; ---------- Readers ----------
;;;
;;; The following functions reads result as lisp objects, and
;;; transform them to json objects recognizable by jsown.

(defun json-name-object (name-obj)
  (json "en" (getf name-obj :en)
        "jp" (getf name-obj :jp)))

(defun json-resistence (obj)
  (json "fire" (getf obj :fire)
        "thunder" (getf obj :thunder)
        "dragon" (getf obj :dragon)
        "water" (getf obj :water)
        "ice" (getf obj :ice)))

(defun json-armor-object (obj)
  (json "name" (json-name-object (getf obj :name))
        "id" (getf obj :id)
        "holes" (getf obj :holes)
        "rare" (if (getf obj :rare)
                   (format nil "~2,'0d" (getf obj :rare))
                   "??")
        "torsoup" (getf obj :torsoup)
        "stuffed" (if (getf obj :stuffed)
                      (getf obj :stuffed)
                      0)
        "maxDef" (getf obj :max-defense)
        "minDef" (getf obj :min-defense)
        "resistence" (json-resistence (getf obj :resistence))
        "jewels" (if (getf obj :jewels)
                     (loop for jewel-plan in (getf obj :jewels)
                        collect (json "name" (json-name-object 
                                              (getf jewel-plan :name))
                                      "num" (getf jewel-plan :quantity)))
                     nil)
        "material" (mapcar #'json-name-object (getf obj :material))))

(defun json-amulet-object (obj)
  (labels ((amulet-name (language obj)
             (let ((name ""))
               (loop for effect in (getf obj :effects)
                  do (setf name (concatenate 'string name 
                                             (getf (getf effect :name) language)
                                             (if (> (getf effect :points) 0)
                                                 " +" " ")
                                             (format nil "~a" (getf effect :points))
                                             "   ")))
               name)))
    (json "holes" (getf obj :holes)
          "id" (getf obj :id)
          "material" ""
          "rare" "??"
          "torsoup" "false"
          "stuffed" 0
          "jewels" nil
          "name" (json "en" (amulet-name :en obj)
                       "jp" (amulet-name :jp obj)))))

;;; ---------- RPC ----------

(def-rpc answer-query (query)
  (let ((query-file (merge-pathnames (format nil "query_cache_~a.lsp"
                                             (hunchentoot:session-id 
                                              current-session))
                                     *working-dir*))
        (output-file (merge-pathnames (format nil "output_~a.lsp"
                                              (hunchentoot:session-id 
                                               current-session))
                                      *working-dir*)))
    (with-open-file (output output-file
                            :direction :output
                            :if-exists :supersede)
      (format output ""))
    (with-open-file (cache query-file
                           :direction :output
                           :if-exists :supersede)
      (format cache "~a~%" query))
    (sb-ext:run-program (namestring *server-binary*)
                        (list (namestring *dataset-path*) 
                              (namestring query-file)
                              (namestring output-file))
                        :output *standard-output* :wait t)
    (with-open-file (input output-file
                           :direction :input)
      (loop for solution = (read input nil nil)
         while solution
         collect (json "gear" (json-armor-object (getf solution :gear))
                       "head" (json-armor-object (getf solution :head))
                       "body" (json-armor-object (getf solution :body))
                       "hands" (json-armor-object (getf solution :hands))
                       "waist" (json-armor-object (getf solution :waist))
                       "feet" (json-armor-object (getf solution :feet))
                       "amulet" (json-amulet-object (getf solution :amulet))
                       "defense" (getf solution :defense)
                       "jewelPlans" 
                       (loop for jewel-plan in (getf solution :jewel-plans)
                          collect (json "active" (loop for active in (getf jewel-plan :active)
                                                    collect (json-name-object active))
                                        "plan" (loop for jewel in 
                                                    (getf jewel-plan :plan)
                                                  collect (json "name" (json-name-object 
                                                                        (getf jewel :name))
                                                                "num" (getf jewel :quantity))))))))))

