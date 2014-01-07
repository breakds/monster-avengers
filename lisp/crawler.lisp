;;;; crawler.lisp
;;;; author: BreakDS <breakds@gmail.com>

(in-package #:breakds.monster-avengers.crawler)


;;; ---------- Skill Crawler ----------

;; (def-struct-wrapper crawl-skill 
;;   ("body .wrapper .container .contents .data_container .data2 table:1 tbody tr:1 td:1" :system-name #'get-content))

(def-struct-wrapper crawl-skill 
  ("body div:1 " :system-name #'identity))
