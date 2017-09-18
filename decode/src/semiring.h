#ifndef SEMIRING_H_
#define SEMIRING_H_

#include "simple_tensor.h"
#include <limits>

#define NEG_INF -std::numeric_limits<double>::infinity()

template <class P> class abstract_semiring {
public:
	virtual ~abstract_semiring()
	{
		
	}

	virtual void mul_by_scalar(P& a, my_double v)
	{
		assert(false);
	}
	
	virtual P zero() = 0;

	virtual P one() = 0;

	virtual P mul(P a, P b, P c) = 0;
	
	virtual P mul(P a, P b) = 0;

	virtual P div(P a, P b) = 0;
	
	virtual P plus(P a, P b) = 0;

	virtual bool equals_zero(P v) = 0;

	virtual bool equals_one(P v) = 0;

	virtual void set_mul(P a, P b, P c, P& out) = 0;
	
	virtual void set_mul(P a, P b, P& out) = 0;
	
	virtual void set_plus(P a, P b, P& out) = 0;

	virtual void plus_mul_xz(P a, P b, P c, P& out) = 0;

	virtual void plus_mul_xy(P a, P b, P c, P& out) = 0;

	virtual void plus_mul_yz(P a, P b, P c, P& out) = 0;

	virtual my_uint type() = 0;

	virtual bool ge(P a, P b) = 0;
	
	typedef enum { sr_type_real = 0, sr_type_max_log = 1, sr_type_tensor = 2 } sr_type;
};

class tensor_semiring : public abstract_semiring<simple_tensor>
{
	// TODO
	// need an operator that does both plus and mul in place
	
public:
	virtual simple_tensor zero()
	{
		return simple_tensor();
	}

	virtual void mul_by_scalar(simple_tensor& a, my_double v)
	{
		a.mul_by_scalar(v);
	}

	virtual simple_tensor one()
	{
		return simple_tensor(1.0);
	}

	virtual simple_tensor mul(simple_tensor a, simple_tensor b, simple_tensor c)
	{
		assert(false);
	}
	
	virtual my_double marginalize_vector(simple_tensor a, simple_tensor b)
	{
		if (a.empty() || b.empty())
		{
			return 0;
		}
		
		assert(a.is_vector());
		assert(b.is_vector());
		
		assert(a.cols() == b.cols());
		
		my_double v = 0;
		
		for (int i=0; i < a.cols(); i++)
		{
			v = v + a.get(i) * b.get(i);
		}
		
		return v;
	}

	virtual my_double marginalize_tensor(simple_tensor a, simple_tensor b, simple_tensor c, simple_tensor abc)
	{
		if (a.empty() || b.empty() || c.empty() || abc.empty())
		{
			return 0;
		}
		
		assert(a.is_vector());
		assert(b.is_vector());
		assert(c.is_vector());
		assert(abc.is_tensor());
		
		assert(a.cols() == abc.slices());
		assert(b.cols() == abc.rows());
		assert(c.cols() == abc.cols());
		
		my_double v = 0;
		
		for (int i=0; i < abc.slices(); i++)
		{
			for (int j=0; j < abc.rows(); j++)
			{
				for (int k=0; k < abc.cols(); k++)
				{
					v = v + abc.get(i, j, k) * a.get(i) * b.get(j) * c.get(k);
				}
			}
		}
		
		return v;
	}

	virtual simple_tensor mul(simple_tensor a, simple_tensor b)
	{
		assert(false);
	}

	virtual simple_tensor div(simple_tensor a, simple_tensor b)
	{
		assert(false);
	}
	
	virtual simple_tensor plus(simple_tensor a, simple_tensor b)
	{
		assert(false);
	}

	virtual bool equals_zero(simple_tensor v)
	{
		return v.equals_zero();
	}

	virtual bool equals_one(simple_tensor v)
	{
		assert(false);
	}

	virtual void set_mul(simple_tensor a, simple_tensor b, simple_tensor c, simple_tensor& out)
	{
		assert(false);
	}
	
	virtual void set_mul(simple_tensor a, simple_tensor b, simple_tensor& out)
	{
		assert(false);
	}
	
	void plus_mul_yz(simple_tensor a, simple_tensor b, simple_tensor c, simple_tensor& out)
	{
		if (a.empty() || (b.empty()) || (c.empty()))
		{
			return;
		}

		assert(a.is_vector());
		
		assert(b.is_vector());

		assert(a.cols() == c.rows());
		
		assert(b.cols() == c.cols());

		out.plus_mul_yz(a, b, c);
	}
	
	void plus_mul_xy(simple_tensor a, simple_tensor b, simple_tensor c, simple_tensor& out)
	{
		if (a.empty() || (b.empty()) || (c.empty()))
		{
			return;
		}

		assert(a.is_vector());
		
		assert(b.is_vector());

		assert(a.cols() == c.slices());
		
		assert(b.cols() == c.rows());

		out.plus_mul_xy(a, b, c);
	}

	void plus_mul_xz(simple_tensor a, simple_tensor b, simple_tensor c, simple_tensor& out)
	{
		if (a.empty() || (b.empty()) || (c.empty()))
		{
			return;
		}

		assert(a.is_vector());
		
		assert(b.is_vector());

		assert(a.cols() == c.slices());
		
		assert(b.cols() == c.cols());

		out.plus_mul_xz(a, b, c);
	}

	void set_plus(simple_tensor a, simple_tensor b, simple_tensor& out)
	{
		assert(false);
	}

	my_uint type()
	{
		return abstract_semiring<simple_tensor>::sr_type_tensor;
	}

	bool ge(simple_tensor a, simple_tensor b)
	{
		assert(false);
	}
};

class real_semiring : public abstract_semiring<my_double> {

public:
	void mul_by_scalar(my_double& a, my_double v)
	{
		a = a * v;
	}

	
	my_double zero() {
		return 0.0;
	}

	my_double one() {
		return 1.0;
	}

	my_double mul(my_double a, my_double b, my_double c) {
		return a*b*c;
	}
	
	my_double mul(my_double a, my_double b)
	{
		return a*b;
	}
	
	my_double div(my_double a, my_double b)
	{
		return a/b;
	}
	
	my_double plus(my_double a, my_double b)
	{
		return a + b;
	}
	
	my_uint type()
	{
		return sr_type_real;
	}
	
	bool equals_zero(my_double v)
	{
		return v == 0.0;
	}

	bool equals_one(my_double v)
	{
		return v == 1.0;
	}

	void set_mul(my_double a, my_double b, my_double c, my_double& out)
	{
		out = a * b * c;
	}

	void set_mul(my_double a, my_double b, my_double& out)
	{
		out = a * b;
	}

	void set_plus(my_double a, my_double b, my_double& out)
	{
		out = a + b;
	}
	
	bool ge(my_double a, my_double b)
	{
		return a >= b;
	}
	
	void plus_mul_xz(double a, double b, double c, double& out) 
	{
		out = out + a*b*c;
	}

	void plus_mul_xy(double a, double b, double c, double& out)
	{
		out = out + a*b*c;		
	}

	void plus_mul_yz(double a, double b, double c, double& out)
	{
		out = out + a*b*c;
	}

};

template <class P>
class max_log_semiring : public abstract_semiring< P >
{
public:
	void mul_by_scalar(P& a, my_double v)
	{
		assert(false);
	}

	P zero()
	{
		return P(-std::numeric_limits<double>::infinity());
	}
	
	P one()
	{
		return P(0);
	}

	P mul(P a, P b, P c) {
		assert(false);
	}
	
	P mul(P a, P b)
	{
		// TODO this is a hack, should connect two trees and multiply the values.
		
		return a;
	}
	
	P div(P a, P b)
	{
		assert(false);
	}
	
	P plus(P a, P b)
	{
		assert(false);
	}
	
	my_uint type()
	{
		return abstract_semiring<P>::sr_type_max_log;
	}

	bool equals_zero(P v)
	{
		return v.v() == -std::numeric_limits<double>::infinity();
	}
	
	virtual void mul_against(P&a, my_double d)
	{
		a.v() += d;
	}


	bool equals_one(P v)
	{
		return v.v() == 0;
	}

	void set_mul(P a, P b, P c, P& out)
	{
		out.v() = a.v() + b.v() + c.v();
		out.set_merge(a, b, c);
	}

	void set_mul(P a, P b, P& out)
	{
		out.v() = a.v() + b.v();
		out.set_merge(a, b);
	}

	void set_plus(P a, P b, P& out)
	{
		if (a.v() >= b.v())
		{
			out = a;
		} else {
			out = b;
		}
	}

	void plus_mul_xz(P a, P b, P c, P& out)
	{
		P mul;
		
		set_mul(a, b, c, mul);
		
		set_plus(out, mul, out);
	}

	void plus_mul_xy(P a, P b, P c, P& out)
	{
		P mul;
		
		set_mul(a, b, c, mul);
		
		set_plus(out, mul, out);
	}

	void plus_mul_yz(P a, P b, P c, P& out)
	{
		P mul;
		
		set_mul(a, b, c, mul);
		
		set_plus(out, mul, out);		
	}

	bool ge(P a, P b)
	{
		return a.v() >= b.v();
	}

};


#endif /*SEMIRING_H_*/
