/*
 * lnum_algo.c
 *
 *  Created on: Nov 7, 2014
 *      Author: rainbow
 */

#include"lnum_algo.h"

//计算最大公约数 gcd(a,b) = gcd(b,a mod b)
large_num lnum_gcd(large_num a, large_num b)
{
	large_num mod_res, gcd_res;
	lnum_init0(&gcd_res);

	if (lnum_iszero(b))
	{
		lnum_mov(&gcd_res, &a);
		return gcd_res; //要复制一份，
	}
	else
	{
		lnum_init0(&mod_res);
		mod_res = lnum_rem_(a, b);
		gcd_res = lnum_gcd(b, mod_res);
		_LNUM_FREE(&mod_res);
		return gcd_res;
	}
}

//扩展 可以计算出 ax+by = gcd(x,y)中的系数a,b
large_num lnum_gcd_ex(large_num a, large_num b, large_num *x, large_num *y)
{
	large_num gcd_res, mod_res, quo_res, step1_res, step2_res, step3_res;
	large_num x1, y1, gcd1;
	lnum_init0(&x1);
	lnum_init0(&y1);
	lnum_init0(&gcd1);
	lnum_init0(&gcd_res);
	lnum_init0(&step1_res);
	lnum_init0(&step2_res);
	lnum_init0(&step3_res);

	if (lnum_iszero(b))
	{
		lnum_init1(x);
		lnum_init0(y);
		lnum_mov(&gcd_res, &a);
		return gcd_res;
	}
	else
	{
		//需要释放的局部变量：mod_res,quo_res,x1,y1,step*_res
		lnum_init0(&mod_res);
		lnum_init0(&quo_res);
		quo_res = lnum_div_(&mod_res, a, b);
		_LNUM_FREE(&quo_res);
		gcd1 = lnum_gcd_ex(b, mod_res, &x1, &y1); //求(d1,x1,y1)
		_LNUM_FREE(&mod_res);

		step1_res = lnum_div_(&mod_res, a, b);
		_LNUM_FREE(&mod_res);
		step2_res = lnum_mul_(step1_res, y1); //(a/b)*y1
		_LNUM_FREE(&step1_res);
		step3_res = lnum_sub_(x1, step2_res);
		_LNUM_FREE(&step2_res);

		lnum_mov(x, &y1);
		lnum_mov(y, &step3_res);

		_LNUM_FREE(&x1);
		_LNUM_FREE(&y1);
		_LNUM_FREE(&step3_res);

		return gcd1;
	}
}

//求解 ax = b (mod n)的解 x
large_num lnum_solve_coresidule_formula(large_num a, large_num b, large_num n)
{
	return a; //还未实现
}

//判断a与b是否mod n同余
int lnum_iscoresidual(large_num a, large_num b, large_num n)
{
	large_num mod_a, mod_b;
	int res;
	lnum_init0(&mod_a);
	lnum_init0(&mod_b);

	mod_a = lnum_rem_(a, n);
	mod_b = lnum_rem_(b, n);

	res = (lnum_cmp(mod_b, mod_a) == 0) ? 1 : 0;

	_LNUM_FREE(&mod_a);
	_LNUM_FREE(&mod_b);
	return res;
}

//计算e^x mod n 基本算法，对各种进制都有用
large_num lnum_mod_exp_sim(large_num e, large_num x, large_num n) //一般方法计算幂摸
{
	large_num pow_res, mod_res;
	lnum_init0(&mod_res);
	pow_res = lnum_pow_(e, x);
	mod_res = lnum_rem_(pow_res, n);

	_LNUM_FREE(&pow_res);

	return mod_res;
}

large_num lnum_mod_exp(large_num e, large_num x, large_num n) //反复平方法计算幂模 见算法导论
{
#if _WHICH_BIT_SIZE == 10
#pragma message("十进制模式下暂时无法使用高效幂摸函数，改掉用通用函数")
	return lnum_mod_exp_sim(e,x,n);
#endif
	large_num c, d, c2, d2, tmp_2, tmp_1;
	lnum_init0(&c);
	lnum_init1(&d);
	lnum_initx(&tmp_2, 2, SIGN_PLUS);
	lnum_init1(&tmp_1);
	size_t bit_len, max_bit_len = sizeof(bit_t) * 8;
	bit_len = lnum_get_bit_count(x) - 1;
	for (int i = bit_len; i >= 0; i--) //i=k downto 0
	{
		c2 = c;
		c = lnum_mul_(c2, tmp_2);
		_LNUM_FREE(&c2);

		d2 = d;
		d = lnum_mul_(d2, d2);
		_LNUM_FREE(&d2);

		d2 = d;
		d = lnum_rem_(d2, n);
		_LNUM_FREE(&d2);

		if ((x.bits[i / max_bit_len] & (1 << (i % max_bit_len))) != 0)
		{
			c2 = c;
			c = lnum_add_(c2, tmp_1);
			_LNUM_FREE(&c2);

			d2 = d;
			d = lnum_mul_(d2, e);
			_LNUM_FREE(&d2);

			d2 = d;
			d = lnum_rem_(d2, n);
			_LNUM_FREE(&d2);
		}
	}
	_LNUM_FREE(&c);
	_LNUM_FREE(&tmp_2);
	_LNUM_FREE(&tmp_1);
	return d;
}

large_num lnum_random_by_bit(unsigned int nr_bit) //生成随机的指定位数的大数
{
	large_num res;
	unsigned int seed = (unsigned int) clock();
	res.length = nr_bit;
	res.sign = SIGN_PLUS;
	res.bits = _LNUM_CALLOC(nr_bit);

	for (unsigned int i = 0; i < nr_bit; i++)
	{
		srand(seed);
		seed++;
		res.bits[i] = (mid_t) rand() % ((mid_t) _BIT_MAX + 1);
	}

	res.bits[res.length - 1] =
			(res.bits[res.length - 1] == 0) ? 1 : res.bits[res.length - 1]; //最高为不能位0
	return res;
}

int lnum_test_prime_simple(large_num n) //费吗定理测试素数
{
	large_num pow_e_res, tmp_2, tmp_1, n_minus_1;
	lnum_initx(&tmp_2, 2, SIGN_PLUS);
	lnum_init1(&tmp_1);
	n_minus_1 = lnum_sub_(n, tmp_1);
	pow_e_res = lnum_mod_exp(tmp_2, n_minus_1, n);
	if (lnum_iscoresidual(pow_e_res, tmp_1, n))
	{
		_LNUM_FREE(&pow_e_res);
		_LNUM_FREE(&tmp_2);
		_LNUM_FREE(&tmp_1);
		_LNUM_FREE(&n_minus_1);
		return 1;
	}
	else
	{
		_LNUM_FREE(&pow_e_res);
		_LNUM_FREE(&tmp_2);
		_LNUM_FREE(&tmp_1);
		_LNUM_FREE(&n_minus_1);
		return 0;
	}
}

static int miller_rabin_witness(large_num a, large_num n) //m-b方法所用的witness方法，详见算法导论
{
	//需要释放：x0 xi
	//2 ^ t * u = n - 1
	size_t t;
	large_num u, xi, xi1;

	large_num tmp_2, tmp_1, n_minus_1, large_2et;
	lnum_initx(&tmp_2, 2, SIGN_PLUS);
	lnum_init1(&tmp_1);
	n_minus_1 = lnum_sub_(n, tmp_1);

	//确定t u
	size_t max_bit_len = sizeof(bit_t) * 8;
	for (t = 1;; t++)
	{
		if ((n_minus_1.bits[t / max_bit_len] & (1 << (t % max_bit_len))) != 0)
		{
			break;
		}
	} //得到了t,用t可以算出u
	lnum_initx(&large_2et, 1 << t, SIGN_PLUS);
	u = lnum_quo_(n_minus_1, large_2et);
	_LNUM_FREE(&large_2et);

	xi = lnum_mod_exp(a, u, n);
	_LNUM_FREE(&u);
	for (size_t i = 1; i <= t; i++)
	{
		xi1 = xi;
		xi = lnum_mod_exp(xi1, tmp_2, n); //xi-1 ^ 2 mod n
		if (lnum_cmp(xi, tmp_1) == 0 && lnum_cmp(xi1, tmp_1) != 0
				&& lnum_cmp(xi1, n_minus_1) != 0)
		{
			_LNUM_FREE(&tmp_1);
			_LNUM_FREE(&tmp_2);
			_LNUM_FREE(&n_minus_1);
			_LNUM_FREE(&xi1);
			_LNUM_FREE(&xi);
			return 1;
		}
		_LNUM_FREE(&xi1);
	}
	if (lnum_cmp(xi, tmp_1) != 0)
	{
		_LNUM_FREE(&tmp_1);
		_LNUM_FREE(&tmp_2);
		_LNUM_FREE(&n_minus_1);
		_LNUM_FREE(&xi);
		return 1;
	}
	_LNUM_FREE(&tmp_1);
	_LNUM_FREE(&tmp_2);
	_LNUM_FREE(&n_minus_1);
	_LNUM_FREE(&xi);
	return 0;
} //1为和数，0为质数

int lnum_test_prime_miller_rabin(large_num n, size_t s) //根据Miller-Rabin方法测试素数
{
#if _WHICH_BIT_SIZE == 10
#pragma message("十进制模式下暂时无法使用这个函数")
	return lnum_test_prime_simple(n);
#endif
	if (lnum_iszero(n) == 1 || (n.length == 1 && n.bits[0] == 1)
			|| (n.bits[0] & 1) == 0) //如果是0或者1,直接返回0,如果是偶数，直接返回0
	{
		return 0;
	}
	large_num random;
	large_num tmp_2, tmp_1, n_minus_1;
	lnum_initx(&tmp_2, 2, SIGN_PLUS);
	lnum_init1(&tmp_1);
	n_minus_1 = lnum_sub_(n, tmp_1);
	for (size_t j = 1; j <= s; j++)
	{

		random = lnum_random_by_bit(n.length);
		random.bits[n.length - 1] = (random.bits[n.length - 1]
				% n.bits[n.length - 1]) + 1;
		if (lnum_cmp(random, n_minus_1) > 0)
		{
			lnum_mov(&random, &n_minus_1);
		}

		if (miller_rabin_witness(random, n))
		{
			_LNUM_FREE(&tmp_1);
			_LNUM_FREE(&tmp_2);
			_LNUM_FREE(&n_minus_1);
			_LNUM_FREE(&random);
			return 0;
		}
	}
	_LNUM_FREE(&tmp_1);
	_LNUM_FREE(&tmp_2);
	_LNUM_FREE(&n_minus_1);
	_LNUM_FREE(&random);
	return 1;
}

void lnum_print(large_num num, char* name) //把指定的数打印导屏幕
{
	char buffer[50000];
	memset(buffer, 0, sizeof(buffer));
	lnum_tostring(buffer, &num);
	printf("%s :%s\tlength : %d\n", name, buffer, num.length);
}

large_num lnum_get_prime_by_bit(int bit_nr, int try_count) //获取指定位数的素数
{
	large_num ran;

	for (int i = 0; i < try_count; i++)
	{
		ran = lnum_random_by_bit(bit_nr / (_BIT_SIZE * 8) + 1);
		ran.bits[0] |= 1;
		ran.bits[ran.length - 1] &= ((1 << (bit_nr % (_BIT_SIZE * 8))) - 1);
		ran.bits[ran.length - 1] |= (1 << ((bit_nr % (_BIT_SIZE * 8)) - 1));

		if (lnum_test_prime_miller_rabin(ran, 50))
		{
			return ran;
		}
		_LNUM_FREE(&ran);
	}
	_LNUM_FREE(&ran);
	lnum_init0(&ran);
	return ran;
}

size_t lnum_get_bit_count(large_num x) //获取大数占用的二进制位数
{
	if (lnum_iszero(x))
		return 0;
	size_t bit_len, max_bit_len = sizeof(bit_t) * 8;
	for (bit_len = max_bit_len - 1; bit_len >= 0; bit_len--)
	{
		if ((x.bits[x.length - 1] & (1 << bit_len)) != 0)
			break;
	}
	bit_len += (x.length - 1) * max_bit_len + 1;
	return bit_len;
}

large_num lnum_from_string(const char *str)
{
	large_num res;
	char* step;
	lnum_init0(&res);
	if (*str == '0' && *(str + 1) == 'x')
	{
		str += 2;
	}
	res.length = ((strlen(str)+1/2)/_BIT_SIZE )+ 1;
	step = res.bits = _LNUM_CALLOC(res.length);

	for(int i = strlen(str) - 1,test=0;i>=0;i--,test++)
	{
		char c = tolower(str[i]);
		if(isalpha(c))
		{
			c =c- 'a'+ '9' + 1;
		}
		if((test & 1) != 0) //是奇数，step该递增
		{
			*step |= ((c - '0')&0x0f)<<4;
			step++;
		}
		else
		{
			*step |= ((c- '0')&0x0f);
		}
	}
	_LNUM_SHRINK(res);
	return res;
}
