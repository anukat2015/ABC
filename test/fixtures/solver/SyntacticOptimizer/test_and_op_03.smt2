(set-logic QF_S)

(declare-fun a () Bool)
(declare-fun b () Bool)
(declare-fun c () Bool)
(declare-fun d () Bool)
(declare-fun e () Bool)
(declare-fun f () Bool)

(assert	(and (= a b)(= b c)(not d) )) 
(assert a)
(assert e)
(assert f)

(check-sat)

