#ifndef Interpolators__1D_InterpolatorBase_hpp
#define Interpolators__1D_InterpolatorBase_hpp

/** @file InterpolatorBase.hpp
  * @brief 
  * @author C.D. Clark III
  * @date 12/24/16
  */

#include <memory>
#include "InterpolatorInterface.hpp"


namespace _1D {

/** @class 
  * @brief A base class for interpolators that provides default implementations of the interface.
  * @author C.D. Clark III
  *
  * This class provides an implementation for the setData method as well as adding a few additional
  * useful methods, including derivative and integral methods.
  */
template<class Real>
class InterpolatorBase : public InterpolatorInterface<Real>
{
  public:
    typedef Eigen::Matrix<Real,Eigen::Dynamic,1> VectorType;
    typedef Eigen::Map<VectorType> MapType;

    // methods required by interface
    virtual void setData( size_t _n, Real *x, Real *y, bool deep_copy = true );

    // additional methods
    virtual Real derivative( Real x );
    virtual Real integral(   Real a, Real b );
    virtual void setData( std::vector<Real> &x, std::vector<Real> &y, bool deep_copy = true );
    virtual void setData( VectorType  &x, VectorType &y, bool deep_copy = true );

  protected:
    VectorType xd, yd;               // data
    std::shared_ptr<MapType> xv, yv; // map view of the data

};

template<class Real>
void
InterpolatorBase<Real>::setData( size_t n, Real *x, Real *y, bool deep_copy )
{
  Real *xp, *yp;
  if( deep_copy )
  {
    xd = Eigen::Map<VectorType>( x, n );
    yd = Eigen::Map<VectorType>( y, n );
    xp = &xd(0);
    yp = &yd(0);
  }
  else
  {
    xp = x;
    yp = y;
  }
  this->xv.reset( new Eigen::Map<VectorType>( xp, n ) );
  this->yv.reset( new Eigen::Map<VectorType>( yp, n ) );
}

template<class Real>
void
InterpolatorBase<Real>::setData( std::vector<Real> &x, std::vector<Real> &y, bool deep_copy )
{
  this->setData( x.size(), x.data(), y.data(), deep_copy );
}

template<class Real>
void
InterpolatorBase<Real>::setData( VectorType  &x, VectorType &y, bool deep_copy )
{
  this->setData( x.size(), &x(0), &y(0), deep_copy );
}


template<class Real>
Real
InterpolatorBase<Real>::derivative( Real x )
{
  if( xv->size() < 1 )
    return 0;

  // simple Finite-Difference approximation
  Real dx = (*xv)(1) - (*xv)(0);
  Real dy = this->operator()( x + dx/2 ) - this->operator()( x - dx/2 );

  return dy/dx;
}

template<class Real>
Real
InterpolatorBase<Real>::integral( Real a, Real b )
{
  if( xv->size() < 1 )
    return 0;

  int sign = 1;

  if( a > b )
  {
    std::swap( a, b );
    sign = -1;
  }

  // simple Trapezoid sum
  Real dx = (*xv)(1) - (*xv)(0);
  int N = (b-a)/dx;
  N = std::max(2,N); // make sure N is at least 2
  dx = (b-a)/(N-1);

  Real sum = 0;
  for(int i = 0; i < N-1; i++)
    sum += 0.5*( this->operator()(a+i*dx) + this->operator()(a+(i+1)*dx) )*dx;

  return sign*sum;
}

}

#endif // include protector