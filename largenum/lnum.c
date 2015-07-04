/*
 * lnum.c
 *
 *  Created on: Nov 3, 2014
 *      Author: rainbow
 */

#include"lnum.h"

#define _CALLOC(bit_length) _LNUM_CALLOC(bit_length)
#define _SHRINK(tmp) _LNUM_SHRINK(tmp)


#define EXTRA_FORMAL_ARGS unsigned int BIT_MAX,unsigned char BIT_SIZE
#define EXTRA_PARAM _BIT_MAX,_BIT_SIZE

#define _FREE(des) _LNUM_FREE(des)
static int errno;

static void lnum_sub_usigned(large_num* des, large_num a, large_num b,
EXTRA_FORMAL_ARGS);
static void lnum_add_usigned(large_num* des, large_num a, large_num b,
EXTRA_FORMAL_ARGS);
static void lnum_mul_usigned_plain(large_num* des, large_num a, large_num b,
EXTRA_FORMAL_ARGS);
static void lnum_div_usigned_plain(large_num* quo, large_num* rem, large_num a,
		large_num b,
		EXTRA_FORMAL_ARGS);

//无符号加法
static void lnum_add_usigned(large_num* des, large_num a, large_num b,
EXTRA_FORMAL_ARGS)
{
	large_num tmp;
	bit_t carry = 0;

	tmp.length = MAX(a.length, b.length) + 1;
	tmp.bits = _CALLOC(tmp.length);
	if (!tmp.bits)
	{
		errno = E_MEM;
		return;
	}

	for (int i = 0; i < tmp.length; i++)
	{
		bit_t tmp_bit;
		bit_t bit_a = i < a.length ? a.bits[i] : 0;
		bit_t bit_b = i < b.length ? b.bits[i] : 0;
		tmp_bit = BIT_MAX - bit_a; //不溢出的最大值
		if (bit_b > tmp_bit - carry) //会溢出
		{
			tmp.bits[i] = bit_b + carry - tmp_bit - 1;
			carry = 1;
		}
		else //不会溢出
		{
			tmp.bits[i] = bit_a + bit_b + carry;
			carry = 0;
		}
	}

	_SHRINK(tmp);
	*des = tmp;
}

//a必须大于b ，无符号剑法
static void lnum_sub_usigned(large_num* des, large_num a, large_num b,
EXTRA_FORMAL_ARGS)
{
	large_num tmp;
	bit_t carry = 0;

	tmp.length = MAX(a.length, b.length);
	tmp.length = a.sign == b.sign ? tmp.length + 1 : tmp.length;
	tmp.bits = _CALLOC(tmp.length);
	if (!tmp.bits)
	{
		errno = E_MEM;
		return;
	}

	for (int i = 0; i < tmp.length; i++)
	{
		bit_t bit_a = i < a.length ? a.bits[i] : 0;
		bit_t bit_b = i < b.length ? b.bits[i] : 0;
		if (bit_b > bit_a - carry) //需要借位
		{
			tmp.bits[i] = BIT_MAX - (bit_b - bit_a) + 1 - carry;
			carry = 1;
		}
		else //不会溢出
		{
			tmp.bits[i] = bit_a - bit_b - carry;
			carry = 0;
		}
	}
	_SHRINK(tmp);
	*des = tmp;
}

//N^2的算法,无符号，与手算方法相同，效率较低
static void lnum_mul_usigned_plain(large_num* des, large_num a, large_num b,
EXTRA_FORMAL_ARGS)
{
	large_num tmp;
	mid_t carry_mul = 0, carry_add = 0;
	mid_t mid_mul, mid_add;

	if (lnum_iszero(a) || lnum_iszero(b))
	{
		tmp.length = 0;
		tmp.bits = NULL;
		tmp.sign = SIGN_PLUS;
		return;
	}

	tmp.length = a.length + b.length;
	tmp.bits = _CALLOC(tmp.length);
	tmp.sign = a.sign == b.sign ? SIGN_PLUS : SIGN_MINUS;

	for (int multiplier = 0; multiplier < b.length; multiplier++) //乘数
	{
		for (int multiplicand = 0; multiplicand < a.length; multiplicand++) //被乘数
		{
			mid_mul = (mid_t) a.bits[multiplicand] * (mid_t) b.bits[multiplier]
					+ carry_mul;
			if (mid_mul > BIT_MAX)  //需要进位
			{
				carry_mul = mid_mul>>(_BIT_SIZE*8);//mid_mul / ((mid_t) BIT_MAX + 1);//这种写法支持10进制
				mid_mul = mid_mul & (_BIT_MAX);//mid_mul % ((mid_t) BIT_MAX + 1);
			}
			else
			{
				carry_mul = 0;
			}
			//加法部分
			mid_add = mid_mul + (mid_t) tmp.bits[multiplier + multiplicand]
					+ carry_add;
			if (mid_add > BIT_MAX)  //需要进位
			{
				carry_add = 1;
				mid_add -= (BIT_MAX + 1);
			}
			else
			{
				carry_add = 0;
			}
			tmp.bits[multiplier + multiplicand] = mid_add;
		}
		//
		if (carry_mul != 0 || carry_add != 0)
		{
			tmp.bits[multiplier + a.length] = carry_mul + carry_add;
			carry_mul = carry_add = 0;

		}
	}
	if (carry_add != 0)
	{
		tmp.bits[tmp.length - 1] = 1;
	}
	_SHRINK(tmp);
	*des = tmp;
}

//计算a/b，,无符号,并且假设a>b ，作为基本算法使用，收算方式计算
static void lnum_div_usigned_plain(large_num* quo, large_num* rem, large_num a,
		large_num b,
		EXTRA_FORMAL_ARGS)
{
	large_num mid_tmp, mid_dividend, mid_rem, mid_quo;
	large_num dividend_backup;
	mid_t high = (mid_t) BIT_MAX + 1, low = 0, mid = (low + high) / 2;
	lnum_init0(&mid_dividend);
	lnum_init0(&dividend_backup);
	_FREE(quo);
	_FREE(rem);
	quo->length = a.length - b.length + 1;
	quo->sign = SIGN_PLUS;
	quo->bits = _CALLOC(quo->length);

	if (lnum_iszero(b))
	{
		errno = E_DZERO;
		return;
	}
	if (lnum_iszero(a))
	{
		return;
	}

	lnum_init1(&mid_tmp); //用来保存试除的数
	lnum_init1(&mid_quo); //试除后得出的一位商
	lnum_init1(&mid_rem); //试除后得到的余数

	mid_dividend.length = b.length;
	mid_dividend.sign = SIGN_PLUS;
	lnum_mov(&dividend_backup, &a);
	//一位一位的试除
	for (int i = quo->length - 1; i >= 0; i--)
	{
		//设置被除数
		mid_dividend.bits = &dividend_backup.bits[i];

		_FREE(&mid_rem);
		//计算商为一位数的情况
		switch (lnum_cmp(mid_dividend, b))
		{
		case 0:
			mid_quo.bits[0] = 1;
			_FREE((&mid_rem))
			;
			lnum_init0(&mid_rem);
			break;
		case -1:
			mid_quo.bits[0] = 0;
			lnum_mov(&mid_rem, &mid_dividend);
			break;
		case 1: //a>b的情况，这才是重头戏
			high = (mid_t) BIT_MAX + 1, low = 0, mid = 2;//(low + high) / 2;小小的hack一下
			while (high != low)
			{
				large_num tmp_res;
				large_num sub_res;
				lnum_init0(&tmp_res);
				mid_tmp.bits[0] = (bit_t) mid;

				lnum_mul(mid_tmp, b, &tmp_res);
				switch (lnum_cmp(tmp_res, mid_dividend))
				{
				case 0: //试除后发现刚好相等
					mid_quo.bits[0] = mid_tmp.bits[0];
					_FREE((&mid_rem))
					;
					lnum_init0(&mid_rem);

					high = low; //这么设置可以让循环退出
					break;
				case -1: //试除所得的商小了，下面测试是不是那种合适的小^_^
					lnum_init0(&sub_res);

					lnum_sub(mid_dividend, tmp_res, &sub_res);
					if (lnum_cmp(sub_res, b) == -1) //比除数小，这个结果正确
					{
						mid_quo.bits[0] = mid_tmp.bits[0];
						_FREE((&mid_rem));
						mid_rem = sub_res;
						high = low;
					}
					else //继续试除
					{
						_FREE((&sub_res));
						low = mid;
						mid = (low + high) / 2;
					}
					break;
				case 1: //试除所得的商大了，不考虑，直接设置mid,high,low进行下一次试除
					high = mid;
					mid = (low + high) / 2;
					break;
				}
				_FREE((&tmp_res));
			}
			break;
		}
		////////////
		//现在mid_quo里是商，mid_rem是用余数
		memcpy(mid_dividend.bits, mid_rem.bits, mid_rem.length * sizeof(bit_t));
		mid_dividend.length =
				1 + (lnum_iszero(mid_rem) == 1 ? 0 : mid_rem.length);

		quo->bits[i] = mid_quo.bits[0];
	}
	*rem = mid_rem;
	_SHRINK((*rem));
	_SHRINK((*quo));
	_FREE(&dividend_backup);
	_FREE(&mid_quo);
	_FREE(&mid_tmp);
}
//有些函数会因为输入错误而无法返回值，这个函数用来返回最后一次出错的原因
int lnum_errno()
{
	return errno;
}
//大数比较，与strcmp返回值有类似含义
int lnum_cmp(large_num a, large_num b) //a>b:1 a<b:-1 a=b:0
{
	int i;
	if (a.length > b.length)
		return 1;
	else if (a.length < b.length)
		return -1;
	else
	{
		for (i = a.length - 1; i >= 0; i--)
		{
			if (a.bits[i] > b.bits[i])
				return 1;
			else if (a.bits[i] < b.bits[i])
				return -1;
			else
				continue;
		}
		return 0;
	}
}
//是否为零
int lnum_iszero(large_num obj)
{
	if (!obj.bits || obj.length == 0 || obj.bits[obj.length - 1] == 0)
		return 1;
	else
		return 0;
}
//拷贝一个大数
void lnum_mov(large_num *des, large_num *src)
{
	if (des->bits == NULL || (des->length < src->length))
	{
		_FREE(des);
		if (!(des->bits = _CALLOC(src->length)))
		{
			errno = E_MEM;
			return;
		}
	}
	memcpy(des->bits, src->bits, src->length * sizeof(bit_t));
	des->length = src->length;
	des->sign = src->sign;
	_SHRINK((*des));
}
//任意数相加，需要传入保存结果的指针
void lnum_add(large_num a, large_num b, large_num* des)
{
	large_num tmp;
	lnum_init0(&tmp);
	_FREE(des);
	if (a.sign == b.sign)
	{
		lnum_add_usigned(&tmp, a, b, EXTRA_PARAM);
		tmp.sign = a.sign;
	}
	else
	{
		int cmp_res = lnum_cmp(a, b);
		switch (cmp_res)
		{
		case 0:
			tmp.bits = NULL;
			tmp.length = 0;
			tmp.sign = SIGN_PLUS;
			break;
		case 1:
			lnum_sub_usigned(&tmp, a, b, EXTRA_PARAM);
			tmp.sign = a.sign;
			break;
		case -1:
			lnum_sub_usigned(&tmp, b, a, EXTRA_PARAM);
			tmp.sign = b.sign;
			break;
		default:
			errno = E_PROG;
			break;
		}
	}
	if (des)
		*des = tmp;
}
//任意数相减，需要传入保存结果的指针
void lnum_sub(large_num a, large_num b, large_num* des)
{
	large_num tmp;
	lnum_init0(&tmp);
	_FREE(des);
	b.sign = b.sign == SIGN_PLUS ? SIGN_MINUS : SIGN_PLUS;
	lnum_add(a, b, &tmp);
	b.sign = b.sign == SIGN_PLUS ? SIGN_MINUS : SIGN_PLUS;
	if (des)
		*des = tmp;
}

//暂时采用N^2的算法 //任意数相乘，需要传入保存结果的指针
void lnum_mul(large_num a, large_num b, large_num* des)
{
	large_num tmp;
	lnum_init0(&tmp);
	_FREE(des);
	lnum_mul_usigned_plain(&tmp, a, b, EXTRA_PARAM);
	if (des)
		*des = tmp;
}
////任意数相除，需要传入保存结果的指针，暂时只能做无符号出发
//这个函数还没有考虑导符号和商是fix还是floor方法取，没有区分modular和remainer
void lnum_div(large_num* rem, large_num a, large_num b, large_num* quo)
{
	large_num tmp;
	lnum_init0(&tmp);
	_FREE(quo);
	_FREE(rem);
	switch (lnum_cmp(a, b))
	{
	case 0:
		lnum_init1(&tmp);
		lnum_init0(rem);
		break;
	case -1:
		lnum_init0(&tmp);
		lnum_mov(rem, &a);
		break;
	case 1:
		lnum_div_usigned_plain(&tmp, rem, a, b, EXTRA_PARAM);
		break;
	}

	if (quo)
		*quo = tmp;
	_SHRINK((*rem));
	_SHRINK((*quo));
}
//快速幂
void lnum_pow(large_num e, large_num x, large_num* des)
{
	large_num tmp, mid, tmp2;
	lnum_init1(&tmp);
	lnum_init1(&tmp2);
	lnum_init0(&mid);

	lnum_mov(&mid, &e);

	if (_WHICH_BIT_SIZE == 10)  //特殊处理
	{
		large_num tmp_x, num_2, quo, rem;
		lnum_init0(&quo);
		lnum_init0(&rem);
		lnum_init0(&tmp_x);
		lnum_mov(&tmp_x, &x);
		lnum_initx(&num_2, 2, SIGN_PLUS);
		while (lnum_iszero(tmp_x) == 0)
		{

			quo = lnum_div_(&rem, tmp_x, num_2);
			if (lnum_iszero(rem) == 0)
			{
				lnum_mov(&tmp2, &tmp);
				lnum_mul(mid, tmp2, &tmp);
			}
			lnum_mov(&tmp2, &mid);
			lnum_mul(tmp2, tmp2, &mid);
			_FREE(&rem);
			lnum_mov(&tmp_x, &quo);

		}
		_FREE(&tmp_x);
		_FREE(&num_2);
	}
	else
	{
		size_t bit_len,max_bit_len = sizeof(bit_t)*8;
		for (bit_len = max_bit_len-1; bit_len >= 0; bit_len--)
		{
			if ((x.bits[x.length - 1] & (1 << bit_len)) != 0)
				break;
		}
		bit_len += (x.length - 1) * max_bit_len + 1;
		for (size_t i = 0; i < bit_len; i++)
		{
			if ((x.bits[i / max_bit_len] & (1 << (i % max_bit_len))) != 0)
			{
				lnum_mov(&tmp2, &tmp);
				lnum_mul(mid, tmp2, &tmp);
			}
			lnum_mov(&tmp2, &mid);
			lnum_mul(tmp2, tmp2, &mid);
		}
	}
	_FREE(&tmp2);
	_FREE(&mid);
	if (des)
		*des = tmp;
}
//不用传入保存结果的大数的函数，结果由函数生成，空间释放由调用者进行
large_num lnum_add_(large_num a, large_num b)
{
	large_num tmp_res;
	lnum_init0(&tmp_res);
	lnum_add(a, b, &tmp_res);
	return tmp_res;
}

large_num lnum_sub_(large_num a, large_num b)
{
	large_num tmp_res;
	lnum_init0(&tmp_res);
	lnum_sub(a, b, &tmp_res);
	return tmp_res;
}

//暂时采用N^2的算法
large_num lnum_mul_(large_num a, large_num b)
{
	large_num tmp_res;
	lnum_init0(&tmp_res);
	lnum_mul(a, b, &tmp_res);
	return tmp_res;
}

large_num lnum_div_(large_num* rem, large_num a, large_num b)
{
	large_num tmp_res;
	lnum_init0(&tmp_res);
	lnum_div(rem, a, b, &tmp_res);
	return tmp_res;
}

large_num lnum_rem_(large_num a, large_num b)
{
	large_num tmp_rem,tmp_quo;
	lnum_init0(&tmp_quo);
	lnum_init0(&tmp_rem);

	tmp_quo = lnum_div_(&tmp_rem,a,b);
	_FREE(&tmp_quo);
	return tmp_rem;
}

large_num lnum_quo_(large_num a, large_num b)
{
	large_num tmp_rem,tmp_quo;
	lnum_init0(&tmp_quo);
	lnum_init0(&tmp_rem);

	tmp_quo = lnum_div_(&tmp_rem,a,b);
	_FREE(&tmp_rem);
	return tmp_quo;
}

large_num lnum_pow_(large_num e, large_num x)
{
	large_num tmp_res;
	lnum_init0(&tmp_res);
	lnum_pow(e, x, &tmp_res);
	return tmp_res;
}
void lnum_init1(large_num* des)
{
	des->bits = _CALLOC(1);
	des->length = 1;
	des->sign = SIGN_PLUS;

	des->bits[0] = 1;
}

void lnum_init2(large_num* des)
{
	des->bits = _CALLOC(1);
	des->length = 1;
	des->sign = SIGN_PLUS;

	des->bits[0] = 2;
}

void lnum_init0(large_num* des)
{
	des->bits = NULL;
	des->length = 0;
	des->sign = SIGN_PLUS;
}

void lnum_initx(large_num* des, unsigned long long x, sign_t sign)
{
	lnum_init0(des);

	des->sign = sign;

	if (_WHICH_BIT_SIZE == 10)
	{
		int i = 0;
		des->bits = _CALLOC(21);
		des->length = 0;
		while (x != 0)
		{
			des->bits[i++] = x % 10;
			x /= 10;
			des->length++;
		}
	}
	else
	{
		des->length = (sizeof(x) / _BIT_SIZE);
		des->bits = _CALLOC(des->length);

		*((unsigned long long*) des->bits) = x;
	}

	_SHRINK((*des));
}
//把数转化为字符串
void lnum_tostring(char* buffer, large_num* des)
{
	if (des->sign == SIGN_MINUS)
	{
		*(buffer++) = '-';
	}
	if (_WHICH_BIT_SIZE != 10)
	{
		*(buffer++) = '0';
		*(buffer++) = 'x';
	}

	for (int i = des->length - 1; i >= 0; i--)
	{
		sprintf(buffer, _FORMAT_CHAR, des->bits[i]);
		char a = des->bits[i];
		buffer += (int) (_BIT_SIZE * 2);
	}
}

