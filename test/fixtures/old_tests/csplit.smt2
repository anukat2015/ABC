(set-logic QF_S)

(declare-fun input () String)
(declare-fun s () String)
(declare-fun s1 () String)
(declare-fun a1 () String)
(declare-fun a2 () String)
(declare-fun a3 () String)
(declare-fun a4 () String)
(declare-fun a5 () String)
(declare-fun a6 () String)
(declare-fun a7 () String)
(declare-fun a8 () String)
(declare-fun a10 () String)
(declare-fun a11 () String)
(declare-fun a12 () String)
(declare-fun a13 () String)
(declare-fun a14 () String)
(declare-fun a15 () String)
(declare-fun a16 () String)
(declare-fun a17 () String)
(declare-fun a18 () String)
(declare-fun a19 () String)
(declare-fun a20 () String)
(declare-fun a21 () String)
(declare-fun a22 () String)
(declare-fun a23 () String)
(declare-fun a24 () String)
(declare-fun a25 () String)
(declare-fun a26 () String)
(declare-fun a27 () String)
(declare-fun a28 () String)
(declare-fun a29 () String)
(declare-fun a30 () String)
(declare-fun a31 () String)
(declare-fun a32 () String)
(declare-fun a33 () String)
(declare-fun var_a34 () String)
(declare-fun a35 () String)

(assert (not (str.in.re input (str.to.re ".*information.*")))) 
(assert (not (str.in.re input (str.to.re ".*z.*")))) 

(assert (= a2 "z"))
(assert (= a1 (str.++ input a2)))

(assert (not (str.in.re s (str.to.re ".*information.*")))) 
(assert (not (str.in.re s (str.to.re ".*z.*")))) 
(assert (= a3 (str.++ a1 s)))

(assert (= a5 "z"))
(assert (= a4 (str.++ a3 a5)))
(assert (= a6 (str.++ a4 s)))

(assert (= a8 "z"))
(assert (= a7 (str.++ a6 a8)))
(assert (= a9 (str.++ a7 s)))

(assert (= (str.len a9) 206))

(assert (= a11 "information"))
(assert (= a10 (str.++ a9 a11)))
(assert (= a9 (str.++ a7 s)))
(assert (not (str.in.re s1 (str.to.re ".*z.*")))) 

(assert (= a12 (str.++ a10 s1)))

(assert (= a14 "z"))
(assert (= a13 (str.++ a12 a14)))
(assert (= a15 (str.++ a13 s1)))

(assert (= a17 "z"))
(assert (= a16 (str.++ a15 a17)))
(assert (= a18 (str.++ a16 s1)))

(assert (= a20 "z"))
(assert (= a19 (str.++ a18 a20)))
(assert (= a21 (str.++ a19 s1)))

(assert (= a23 "z"))
(assert (= a22 (str.++ a21 a23)))
(assert (= a24 (str.++ a22 s1)))

(assert (= a26 "z"))
(assert (= a25 (str.++ a24 a26)))
(assert (= a27 (str.++ a25 s1)))

(assert (= a29 "z"))
(assert (= a28 (str.++ a27 a29)))
(assert (= a30 (str.++ a28 s1)))

(assert (= a32 "z"))
(assert (= a31 (str.++ a30 a32)))
(assert (= a33 (str.++ a31 s1)))

(assert (= a35 "z"))
(assert (= var_a34 (str.++ a33 a35)))

(assert (= (str.len var_a34) 629))

(check-sat)
