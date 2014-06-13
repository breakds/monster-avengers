;;;; encoding.lisp
;;;; Author: BreakDS <breakds@gmail.com>
;;;; 
;;;; Description: Component of the package armor-up that provides
;;;; subroutines to encode/decode skill signatures and hole signatures

(in-package #:breakds.monster-avengers.armor-up)



;;; ---------- Hole Signatures ----------
;;; A hole signature is a 3-length list such as (2 3 1), where
;;; first element -> number of 1-holes
;;; second element -> number of 2-holes
;;; third element -> number of 3-holes

(declaim (inline encode-hole-sig))
(defun encode-hole-sig (hole-sig)
  "This function encode a hole signature into a 64-bit unsigned
  integer. Out of the 64 bits, the first 12 bits are used to encode
  the hole signature, where the number of each type of hole have 4
  bits for itself."
  (the (unsigned-byte 64)
       (logior (ldb (byte 4 0) 
                    (the (signed-byte 8) (nth 0 hole-sig)))
               (ash (ldb (byte 4 0) 
                         (the (signed-byte 8) (nth 1 hole-sig))) 4)
               (ash (ldb (byte 4 0) 
                         (the (signed-byte 8) (nth 2 hole-sig))) 8))))

(declaim (inline decode-hole-sig))
(defun decode-hole-sig (key)
  "The inverse function of encode-hole-sig."
  (declare (type (unsigned-byte 64) key))
  (list (logand key #b1111)
        (logand (ash key -4) #b1111)
        (logand (ash key -8) #b1111)))



;;; ---------- Skill Signatures ----------
;;; A skill signature is an arbitrary long list such as (5 -3) and (-1
;;; -2 6). Each number stands for the total points of a skill-system.

(declaim (inline encode-skill-sig))
(defun encode-skill-sig (skill-sig)
  "This function encode a skill signature into a 64-bit unsigned
  integer. Out of the 64 bits, the last 52 bits are used to encode the
  hole signature, where the points of each skill gets 6 bit in order."
  (let ((result (the (unsigned-byte 64) 0)))
    (declare (type (unsigned-byte 64) result))
    (loop 
       for points in skill-sig
       for offset from 12 by 6
       do (setf result 
                (logior result
                        (ash (ldb (byte 6 0) points) offset))))
    result))

(declaim (inline decode-skill-sig))
(defun decode-skill-sig (key n)
  "The semi inverse function of encode-skill-sig. This is different
  from an actual inverse function of encode-skill-sig as it represent
  negative number as its complement code."
  (declare (type (unsigned-byte 64) key))
  (loop
     for i below n
     for offset from 12 by 6
     collect (logand (ash key (- offset)) #b111111)))

(declaim (inline decode-skill-sig-full))
(defun decode-skill-sig-full (key n)
  "The actual inverse function of encode-skill-sig."
  (declare (type (unsigned-byte 64) key))
  (loop
     for i below n
     for offset from 12 by 6
     collect (let ((x (logand (ash key (- offset)) #b111111)))
               (if (logbitp 5 x)
                   (dpb x (byte 6 0) -1)
                   x))))

(declaim (inline decode-skill-sig-at))
(defun decode-skill-sig-at (key i)
  "Decode the points of i-th skill from the key."
  (declare (type (unsigned-byte 64) key))
  (let* ((offset (+ 12 (* 6 i)))
	 (x (logand (ash key (- offset)) #b111111)))
    (if (logbitp 5 x)
	(dpb x (byte 6 0) -1)
	x)))


;;; ---------- Full Coding ----------
(declaim (inline encode-sig))
(defun encode-sig (hole-sig skill-sig)
  (the (unsigned-byte 64)
       (logior (encode-hole-sig hole-sig)
               (encode-skill-sig skill-sig))))

(declaim (inline decode-sig))
(defun decode-sig (key n)
  (declare (type (unsigned-byte 64) key))
  (values (decode-hole-sig key)
          (decode-skill-sig key n)))

(declaim (inline decode-sig-full))
(defun decode-sig-full (key n)
  (declare (type (unsigned-byte 64) key))
  (values (decode-hole-sig key)
          (decode-skill-sig-full key n)))

(declaim (inline hole-part))
(defun hole-part (key)
  (declare (type (unsigned-byte 64) key))
  (the (unsigned-byte 64) (ldb (byte 12 0) key)))


;;; ---------- Coding Utilities ----------

(declaim (inline encode-armor))
(defun encode-armor (armor-piece required-effects)
  (let ((hole-sig (make-list 3 :initial-element 0))
        (skill-sig (loop for (id points) in required-effects
                      collect (aif (assoc id (armor-effects armor-piece))
                                   (cadr it)
                                   0))))
    (when (> (armor-holes armor-piece) 0)
      (incf (nth (1- (armor-holes armor-piece)) hole-sig)))
    (encode-sig hole-sig skill-sig)))


;;; The following two functions, gen-skill-mask and
;;; is-satisfied-skill-key are used to test whether a skill-key is
;;; said to be satisfied. KEY-A (a1 b1 c1) is said to satisfy the
;;; requirement represented by KEY-B (a2 b2 c2) if a1 < b1, a2 < b2
;;; and a3 < b3 hold simultaneously. The way we test for such
;;; satisification is to calculate the inverse of KEY-B (INV-KEY-B) as
;;; (-a2 -b2 -c2), and test whether (encoded-+ KEY-A INV-KEY-V) has
;;; every skill chunk positive. This is equivalent to test the sign
;;; bits of the first N (length of KEY-A) chunks of (encoded-+ KEY-A
;;; INV-KEY-V).
(defun gen-skill-mask (n)
  "Generate the mask that test for the positivity of the first N
  skills in a skill-key."
  (let ((result (the (unsigned-byte 64) 0)))
    (declare (type (unsigned-byte 64) result))
    (loop 
       for offset from 17 to 62 by 6
       for i below n
       do (setf (ldb (byte 1 offset) result) 1))
    result))

(declaim (inline is-satisfied-skill-key))
(defun is-satisfied-skill-key (key mask)
  "Test whether KEY (which is usually a sum of test skill keys and
  inverse requirement key) has all positive skill chunks, where the
  number of chunks being tested is dictated by MASK."
  (declare (type (unsigned-byte 64) key))
  (declare (type (unsigned-byte 64) mask))
  #f3
  (zerop (logand mask key)))

;;; ---------- Coding Arithmetics ----------

(declaim (inline encoded-+))
(defun encoded-+ (key-a key-b)
  (declare (type (unsigned-byte 64) key-a))
  (declare (type (unsigned-byte 64) key-b))
  (let ((result (the (unsigned-byte 64)
                     (+ (ldb (byte 12 0) key-a)
                        (ldb (byte 12 0) key-b)))))
    (declare (type (unsigned-byte 64) result))
    (loop 
       for offset from 12 to 57 by 6
       do (setf (ldb (byte 6 offset) result)
                (+ (ldb (byte 6 offset) key-a)
                   (ldb (byte 6 offset) key-b))))
    result))

(declaim (inline encoded-skill-+-base))
(defun encoded-skill-+-base (key-a key-b)
  (declare (type (unsigned-byte 64) key-a))
  (declare (type (unsigned-byte 64) key-b))
  #f3
  (let ((result (the (unsigned-byte 64) 0)))
    (declare (type (unsigned-byte 64) result))
    (loop 
       for offset from 12 to 57 by 6
       do (setf (ldb (byte 6 offset) result)
                (+ (ldb (byte 6 offset) key-a)
                   (ldb (byte 6 offset) key-b))))
    result))

(defmacro encoded-skill-+ (&rest keys)
  (reduce (lambda (y x) 
	    `(encoded-skill-+-base ,x ,y))
	  keys))

(declaim (inline replace-skill-key-at))
(defun replace-skill-key-at (key n value)
  (declare (type (unsigned-byte 64) key))
  (let ((result key)
	(offset (+ 12 (* 6 n))))
    (declare (type (unsigned-byte 64) result))
    (setf (ldb (byte 6 offset) result)
	  (ldb (byte 6 0) value))
    result))



