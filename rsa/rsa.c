#include"rsa.h"

int rsa_get_key(unsigned int bit, rsa_key *pri, rsa_key *pub)
{
	if (bit < 32)
	{
		//return 0;
	}
	large_num p, q, n, fn, p_minus_1, q_minux_1, tmp_1, tmp_gcd, tmp_y;
	//fn为n的欧拉函数
	large_num e, d;
	start:
	lnum_initx(&e, 65537, SIGN_PLUS); //e可以固定位65537
	lnum_init1(&tmp_1);
	p = lnum_get_prime_by_bit(bit / 3, 2000);
	do
	{
		q = lnum_get_prime_by_bit(bit - bit / 3, 2000);
	} while (lnum_cmp(p, q) == 0);


	p_minus_1 = lnum_sub_(p, tmp_1);
	q_minux_1 = lnum_sub_(q, tmp_1);

	n = lnum_mul_(p, q);
	fn = lnum_mul_(p_minus_1, q_minux_1);

	lnum_init0(&tmp_y);
	lnum_init0(&d);
	tmp_gcd = lnum_gcd_ex(e, fn, &d, &tmp_y);
	_LNUM_FREE(&tmp_y);
	_LNUM_FREE(&tmp_gcd);

	if (d.sign == SIGN_MINUS)
		goto start;

	pub->e_or_d = e;
	pub->n = n;
	pub->type = PUBLIC;

	pri->e_or_d = d;
	pri->n.bits = 0;
	lnum_mov(&(pri->n), &n);
	pri->type = PRIVATE;

	_LNUM_FREE(&tmp_1);
	_LNUM_FREE(&p);
	_LNUM_FREE(&q);
	_LNUM_FREE(&p_minus_1);
	_LNUM_FREE(&q_minux_1);
	_LNUM_FREE(&fn);
	return 1;
}

int rsa_encode(char content[], char res[], int len, rsa_key key)
{
	large_num charac,mid_res;
	lnum_init0(&charac);
	if(len>lnum_get_bit_count(key.n))//内容太长，无法加密
	{
		return 0;
	}
	charac.bits = content;
	charac.length = len / _BIT_SIZE + 1;
	charac.sign = SIGN_PLUS;

	_LNUM_SHRINK(charac);
	mid_res = lnum_mod_exp(charac,key.e_or_d,key.n);
	memcpy(res,mid_res.bits,sizeof(bit_t)*mid_res.length);

	_LNUM_FREE(&mid_res);
	return 1;
}

int rsa_decode(char content[], char res[], int len, rsa_key key)
{
	large_num charac,mid_res;
	lnum_init0(&charac);
	if(len>lnum_get_bit_count(key.n))//内容太长，无法加密
	{
		return 0;
	}
	charac.bits = content;
	charac.length = len / _BIT_SIZE + 1;
	charac.sign = SIGN_PLUS;

	_LNUM_SHRINK(charac);
	mid_res = lnum_mod_exp(charac,key.e_or_d,key.n);

	memcpy(res,mid_res.bits,sizeof(bit_t)*mid_res.length);

	_LNUM_FREE(&mid_res);
	return 1;
}
