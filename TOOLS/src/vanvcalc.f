      subroutine vanvcalc( nt, xt, c ) 
C          nt :  Number of Nodes (OUTPUT: INTEGER)  
C          xt :  Node Points     (OUTPUT: REAL*8)  
C          c  :  Spline Coeff    (OUTPUT: REAL*8)  
C
      external func
      real*8   x(1024), z(1024), w(1024), c(64)
      real*8   xt(64), r(1024), vw(1024), err, rnor
      integer  i, n, icon, ivw(8192)
C
      do 100 i=1,511
        x(i+512) = dble(i)/512.0
        w(i+512) = 1.0
        call DAQE(0.0, x(i+512), func, 1.0e-8, 0.0, 20, 641,
     1           z(i+512), err, n, icon)
100   continue
C
      x(1024) = 1.0
      z(1024) = 1.0
      w(1024) = 1.0

      do 200 i=1,512
        x(i) = -x(1025 - i)
        z(i) = -z(1025 - i)
        w(i) = 1.0
200   continue
C
      nt = 16
      xt(1) = -1.0
      xt(2) = -0.968
      xt(3) = -0.936
      xt(4) = -0.872
      xt(5) = -0.768
      xt(6) = -0.512
      xt(7) = -0.256
      xt(8) = -0.128
      xt(9) = 0.128
      xt(10)= 0.256
      xt(11)= 0.512
      xt(12)= 0.768
      xt(13)= 0.872
      xt(14)= 0.936
      xt(15)= 0.968
      xt(16)= 1.000
C      
      call DBSC1(z, x, w, 1024, 3, xt, nt, c,
     1          r, rnor, vw, ivw, icon)
C
      return
      end
C
      double precision function func(x)
      real*8 x
C
      func = ( 4.0*exp(-1.0/(1.0+x))
     1       + 4.0*exp(-1.0/(1.0-x))
     2       + 8.0*exp(-1.0/(2.0-2.0*x*x))
     3       + 2.0 ) / sqrt(1.0 - x*x)/ 11.11647701263427
      return
      end
C
