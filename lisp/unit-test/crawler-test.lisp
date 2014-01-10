;;;; crawler-test.lisp
;;;; Unit tests for crawler-test.lisp
;;;; Author: BreakDS <breakds@gmail.com>

(in-package #:breakds.monster-avengers.crawler-test)

(defsuite* (test-all :in root-suite
                     :documentation "unit tests for mh4 crawler"))

(deftest crawl-skill-system-test ()
  (struct-equal (crawl-skill-system (html-from-uri "http://wiki.mh4g.org/ida/64291.html"))
                '(:SYSTEM-NAME "攻撃" :SKILLS
                  ((:DESCRIPTION "攻撃力-20" :POINTS -20 :NAME "攻撃力DOWN【大】" :OBJ T)
                   (:DESCRIPTION "攻撃力-15" :POINTS -15 :NAME "攻撃力DOWN【中】" :OBJ T)
                   (:DESCRIPTION "攻撃力-10" :POINTS -10 :NAME "攻撃力DOWN【小】" :OBJ T)
                   (:DESCRIPTION "攻撃力+10" :POINTS 10 :NAME "攻撃力UP【小】" :OBJ T)
                   (:DESCRIPTION "攻撃力+15" :POINTS 15 :NAME "攻撃力UP【中】" :OBJ T)
                   (:DESCRIPTION "攻撃力+20" :POINTS 20 :NAME "攻撃力UP【大】" :OBJ T)
                   (:DESCRIPTION "攻撃力がさらに上がる" :POINTS 25 :NAME "攻撃力UP【超】" :OBJ T))
                  :OBJ T)))
