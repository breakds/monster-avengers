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
          "jewels" nil
          "name" (json "en" (amulet-name :en obj)
                       "jp" (amulet-name :jp obj)))))

(defun json-summary (obj)
  (loop for effects in obj
     collect (json "name" (json-name-object (getf effects :name))
                   "points" (getf effects :points)
                   "active" (json-name-object (getf effects :active)))))

;;; ---------- RPC ----------

(def-rpc answer-query (query)
  (let ((session-id (hunchentoot:session-id hunchentoot:*session*))
        (local-session hunchentoot:*session*))
    (hunchentoot:log-message* :info "session: ~a query start."
                              session-id)
    ;; Kill alive explore processes
    (let ((explore-process (hunchentoot:session-value :explore-process local-session)))
      (when (and explore-process
                 (sb-ext:process-alive-p explore-process))
        (sb-ext:process-kill explore-process 9)))
    (let ((query-file (merge-pathnames (format nil "query_cache_~a.lsp"
                                               session-id)
                                       *working-dir*))
          (output-file (merge-pathnames (format nil "output_~a.lsp"
                                                session-id)
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
                            collect (json "summary" (json-summary (getf jewel-plan :summary))
                                          "bodyPlan" (loop for jewel in 
                                                          (getf jewel-plan :body-plan)
                                                        collect (json "name" (json-name-object 
                                                                              (getf jewel :name))
                                                                      "num" (getf jewel :quantity)))
                                          "bodyStuffed" (getf jewel-plan :body-stuffed)
                                          "plan" (loop for jewel in 
                                                      (getf jewel-plan :plan)
                                                    collect (json "name" (json-name-object 
                                                                          (getf jewel :name))
                                                                  "num" (getf jewel :quantity)))))))))))

(def-rpc answer-explore (query init-call)
  (let ((session-id (hunchentoot:session-id hunchentoot:*session*))
        (local-session hunchentoot:*session*))
    (hunchentoot:log-message* :info "session: ~a explore start."
                              session-id)
  (let ((query-file (merge-pathnames (format nil "explore_query_cache_~a.lsp"
                                             session-id)
                                     *working-dir*))
        (output-file (merge-pathnames (format nil "explore_output_~a.lsp"
                                              session-id)
                                      *working-dir*)))
    (when (equal init-call "true")
      (with-open-file (cache query-file
                             :direction :output
                             :if-exists :supersede)
        (format cache "~a~%" query))

      ;; Empty the output file.
      (with-open-file (output output-file
                              :direction :output
                              :if-exists :supersede)
        (format output ""))
      (setf (hunchentoot:session-value :explore-process local-session)
            (sb-ext:run-program (namestring *explore-binary*)
                                (list (namestring *dataset-path*) 
                                      (namestring query-file)
                                      (namestring output-file))
                                :wait nil))
      (sleep 0.4))
    (let* ((explore-process (hunchentoot:session-value :explore-process local-session))
           (finished (not (sb-ext:process-alive-p explore-process)))
           (current-result  (with-open-file (input output-file
                                                   :direction :input)
                              (loop for entry = (read input nil nil) 
                                 while entry
                                 collect entry)))
           (size (length current-result)))
      (json "finished" (if finished "true" "false")
            "result" (mapcar #`,(car x1)
                             (remove-if #`,(eq (second x1) :fail)
                                        current-result))
            "percentage" (ceiling (* size 100) 
                                  (length *skill-systems*)))))))


