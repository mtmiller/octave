      SUBROUTINE ODESSA_INTDY (T, K, YH, NYH, DKY, IFLAG)
      IMPLICIT DOUBLE PRECISION (A-H,O-Z)
      DIMENSION YH(NYH,1), DKY(1)
      COMMON /ODE001/ ROWND, ROWNS(173),
     2   RDUM1(38),H, RDUM2(2), HU, RDUM3, TN, UROUND,
     3   IOWND(14), IOWNS(4),
     4   IDUM1(8), L, IDUM2,
     5   IDUM3(5), N, NQ, IDUM4(4)
C-----------------------------------------------------------------------
C ODESSA_INTDY COMPUTES INTERPOLATED VALUES OF THE K-TH DERIVATIVE OF THE
C DEPENDENT VARIABLE VECTOR Y, AND STORES IT IN DKY.  THIS ROUTINE
C IS CALLED WITHIN THE PACKAGE WITH K = 0 AND T = TOUT, BUT MAY
C ALSO BE CALLED BY THE USER FOR ANY K UP TO THE CURRENT ORDER.
C (SEE DETAILED INSTRUCTIONS IN THE USAGE DOCUMENTATION.)
C-----------------------------------------------------------------------
C THE COMPUTED VALUES IN DKY ARE GOTTEN BY INTERPOLATION USING THE
C NORDSIECK HISTORY ARRAY YH.  THIS ARRAY CORRESPONDS UNIQUELY TO A
C VECTOR-VALUED POLYNOMIAL OF DEGREE NQCUR OR LESS, AND DKY IS SET
C TO THE K-TH DERIVATIVE OF THIS POLYNOMIAL AT T.
C THE FORMULA FOR DKY IS..
C             Q
C  DKY(I)  =  SUM  C(J,K) * (T - TN)**(J-K) * H**(-J) * YH(I,J+1)
C            J=K
C WHERE  C(J,K) = J*(J-1)*...*(J-K+1), Q = NQCUR, TN = TCUR, H = HCUR.
C THE QUANTITIES  NQ = NQCUR, L = NQ+1, N = NEQ, TN, AND H ARE
C COMMUNICATED BY COMMON.  THE ABOVE SUM IS DONE IN REVERSE ORDER.
C IFLAG IS RETURNED NEGATIVE IF EITHER K OR T IS OUT OF BOUNDS.
C-----------------------------------------------------------------------
      IFLAG = 0
      IF (K .LT. 0 .OR. K .GT. NQ) GO TO 80
      TP = TN - HU*(1.0D0 + 100.0D0*UROUND)
      IF ((T-TP)*(T-TN) .GT. 0.0D0) GO TO 90
C
      S = (T - TN)/H
      IC = 1
      IF (K .EQ. 0) GO TO 15
      JJ1 = L - K
      DO 10 JJ = JJ1,NQ
 10     IC = IC*JJ
 15   C = DBLE(IC)
      DO 20 I = 1,NYH
 20     DKY(I) = C*YH(I,L)
      IF (K .EQ. NQ) GO TO 55
      JB2 = NQ - K
      DO 50 JB = 1,JB2
        J = NQ - JB
        JP1 = J + 1
        IC = 1
        IF (K .EQ. 0) GO TO 35
        JJ1 = JP1 - K
        DO 30 JJ = JJ1,J
 30       IC = IC*JJ
 35     C = DBLE(IC)
        DO 40 I = 1,NYH
 40       DKY(I) = C*YH(I,JP1) + S*DKY(I)
 50     CONTINUE
      IF (K .EQ. 0) RETURN
 55   R = H**(-K)
      DO 60 I = 1,NYH
 60     DKY(I) = R*DKY(I)
      RETURN
C
 80   CALL XERR('ODESSA_INTDY--  K (=I1) ILLEGAL',
     1  51, 1, 1, K, 0, 0, ZERO,ZERO)
      IFLAG = -1
      RETURN
 90   CALL XERR ('ODESSA_INTDY--  T (=R1) ILLEGAL',
     1   52, 1, 0, 0, 0, 1, T, ZERO)
      CALL XERR('T NOT IN INTERVAL TCUR - HU (= R1) TO TCUR (=R2)',
     1   52, 1, 0, 0, 0, 2, TP, TN)
      IFLAG = -2
      RETURN
C------------------ END OF SUBROUTINE ODESSA_INTDY -----------------------
      END
