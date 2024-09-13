      subroutine lev4corr(rho, rho4) 
c
      external corr
      real*8   rho, rho4, epsa, epsr, err, area0, area1, result
      integer  nmin, nmax, ncalc, icon
c
      area0= 0.00
      area1= rho
      epsa = 1.0e-6
      epsa = 1.0e-6
      nmin = 20
      nmax = 641
c
      call daqe(area0, area1, corr, epsa, epsr, nmin, nmax,
     1          result, err, ncalc, icon)
      rho4 = result
c
      return
      end
c
c
c
      real*8 function corr(r)
      real*8 r
c
      corr = (  4.0*exp(-1.0/(1.0+r))
     1        + 4.0*exp(-1.0/(1.0-r))
     2        + 8.0*exp(-0.5/(1.0-r*r))
     3        + 2.0 )
     4     / ( sqrt(1.0 - r*r) )
c
      return
      end
