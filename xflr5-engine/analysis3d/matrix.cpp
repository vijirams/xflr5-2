/****************************************************************************

	Techwing Application

	Copyright (C) Andre Deperrois adeperrois@xflr5.com

	All rights reserved.

*****************************************************************************/

#include <QtCore>
#include "matrix.h"
#include "analysis3d_params.h"

/** Transposes in place a 3x3 matrix */
void transpose33(double *l)
{
	double temp;
	temp=l[1];   l[1]=l[3];  l[3]=temp;
	temp=l[2];   l[2]=l[6];  l[6]=temp;
	temp=l[5];   l[5]=l[7];  l[7]=temp;
}

/** Inverts in place a 3x3 matrix */
bool invert33(double *l)
{
	double mat[9];
	double det;

	memcpy(mat,l,sizeof(mat));

	det  = mat[0] *(mat[4] * mat[8] - mat[5]* mat[7]);
	det -= mat[1] *(mat[3] * mat[8] - mat[5]* mat[6]);
	det += mat[2] *(mat[3] * mat[7] - mat[4]* mat[6]);
	if(fabs(det)<PRECISION) return false;

	* l     = (mat[4] * mat[8] - mat[5] * mat[7])/det;
	*(l+1)  = (mat[2] * mat[7] - mat[1] * mat[8])/det;
	*(l+2)  = (mat[1] * mat[5] - mat[2] * mat[4])/det;

	*(l+3)  = (mat[5] * mat[6] - mat[3] * mat[8])/det;
	*(l+4)  = (mat[0] * mat[8] - mat[2] * mat[6])/det;
	*(l+5)  = (mat[2] * mat[3] - mat[0] * mat[5])/det;

	*(l+6)  = (mat[3] * mat[7] - mat[4] * mat[6])/det;
	*(l+7)  = (mat[1] * mat[6] - mat[0] * mat[7])/det;
	*(l+8)  = (mat[0] * mat[4] - mat[1] * mat[3])/det;

	return true;
}


/**
* Method for the comparison of two complex number
*@param a first complex number
*@param b second complex number
*@return 1 if Real(a) > Real(b), -1 if Real(a)<Real(b); if Real(a)=Real(b), returns 1 if Imag(a)>Image(b), -1 otherwise.
*/
int Compare(complex<double> a, complex<double>b)
{
	if(a.real()>b.real())       return  1;
	else if (a.real()<b.real()) return -1;
	else
	{	//same real part
		if(a.imag()>b.imag())         return  1;
		else if (a.imag()<b.imag())   return -1;
		else return 0;
	}
	return 0;
}


/**
* Bubble sort algorithm for complex numbers
*@param array the array of complex numbers to sort
*@param ub the size of the aray
*/
void ComplexSort(complex<double>*array, int ub)
{
	int indx, indx2;
	complex<double> temp, temp2;
	int flipped;

	if (ub <= 1) return;

	indx = 1;
	do
	{
		flipped = 0;
		for (indx2 = ub - 1; indx2 >= indx; --indx2)
		{
			temp  = array[indx2];
			temp2 = array[indx2 - 1];
			if (Compare(temp2, temp) > 0)
			{
				array[indx2 - 1] = temp;
				array[indx2] = temp2;
				flipped = 1;
			}
		}
	} while ((++indx < ub) && flipped);
}



/**
 * @brief Av33 performs the product of the 3x3 matrix A and vector v. Stores the result in array p.
 * @param A a pointer to the matrix
 * @param v the input vector;
 * @param p the output vector;
 */
void AV33(double *A, double *v, double *p)
{
	p[0] = A[0]*v[0] + A[1]*v[1] + A[2]*v[2];
	p[1] = A[3]*v[0] + A[4]*v[1] + A[5]*v[2];
	p[2] = A[6]*v[0] + A[7]*v[1] + A[8]*v[2];
}



/**
* Solves a linear system using Gauss partial pivot method
*@param A a pointer to the single dimensionnal array of double values. Size is n².
*@param n the size of the square matrix
*@param B a pointer to the array of m RHS
*@param m the number of RHS arrays to solve
*@param pbCancel a pointer to the boolean variable which holds true if the operation shold be interrupted.
*@return true if the problem was successfully solved.
*/
bool Gauss(double *A, int n, double *B, int m, bool *pbCancel)
{
	int row, i, j, pivot_row, k;
	double max, dum, *pa, *pA, *A_pivot_row;
	// for each variable find pivot row and perform forward substitution
	pa = A;
	for (row = 0; row < (n - 1); row++, pa += n)
	{
//		qApp->processEvents();
		if(*pbCancel) return false;
		//  find the pivot row
		A_pivot_row = pa;
		max = qAbs(*(pa + row));
		pA = pa + n;
		pivot_row = row;
		for (i=row+1; i < n; pA+=n, i++)
		{
			if ((dum = qAbs(*(pA+row))) > max)
			{
				max = dum;
				A_pivot_row = pA;
				pivot_row = i;
			}
		}
		if (max <= 0.0)
		{
			return false;                // the matrix A is singular
		}
			// and if it differs from the current row, interchange the two rows.

		if (pivot_row != row)
		{
			for (i = row; i < n; i++)
			{
				dum = *(pa + i);
				*(pa + i) = *(A_pivot_row + i);
				*(A_pivot_row + i) = dum;
			}
			for(k=0; k<m; k++)
			{
				dum = B[row+k*n];
				B[row+k*n] = B[pivot_row+k*n];
				B[pivot_row+k*n] = dum;
			}
		}

		// Perform forward substitution
		for (i = row+1; i<n; i++)
		{
			pA = A + i * n;
			dum = - *(pA + row) / *(pa + row);
			*(pA + row) = 0.0;
			for (j=row+1; j<n; j++) *(pA+j) += dum * *(pa + j);
			for (k=0; k<m; k++)
				B[i+k*n] += dum * B[row+k*n];
		}
	}
	// Perform backward substitution

	pa = A + (n - 1) * n;
	for (row = n - 1; row >= 0; pa -= n, row--)
	{
//		qApp->processEvents();
		if(*pbCancel) return false;

		if ( *(pa + row) == 0.0 )
		{
			return false;           // matrix is singular
		}
		dum = 1.0 / *(pa + row);
		for ( i = row + 1; i < n; i++) *(pa + i) *= dum;
		for(k=0; k<m; k++) B[row+k*n] *= dum;
		for ( i = 0, pA = A; i < row; pA += n, i++)
		{
			dum = *(pA + row);
			for ( j = row + 1; j < n; j++) *(pA + j) -= dum * *(pa + j);
			for(k=0; k<m; k++)
				B[i+k*n] -= dum * B[row+k*n];
		}
	}
	return true;
}




/**
*Inverts a complex 4x4 matrix
*@param ain in input, a pointer to a one-dimensional array holding the 16 complex values of the input matrix
*@param aout in output, a pointer to a one-dimensional array holding the 16 complex values of the inverted matrix
*@return if the inversion was successful
*/
bool Invert44(complex<double> *ain, complex<double> *aout)
{
	//small size, use the direct method
	int i,j;
	complex<double> det;
	double sign;

	det = det44(ain);

	if(abs(det)<PRECISION) return false;

	for(i=0; i<4; i++)
	{
		for(j=0; j<4; j++)
		{
			sign = pow(-1.0,i+j);
			aout[4*j+i] = sign * cofactor44(ain, i, j)/det;
		}
	}
	return true;
}


/**
*Returns the determinant of a 3x3 matrix
*@param aij a pointer to a one-dimensional array holding the 9 double values of the matrix
*@return the matrix's determinant
*/
double det33(double *aij)
{
	//returns the determinant of a 3x3 matrix

	double det;

	det  = aij[0]*aij[4]*aij[8];
	det -= aij[0]*aij[5]*aij[7];

	det -= aij[1]*aij[3]*aij[8];
	det += aij[1]*aij[5]*aij[6];

	det += aij[2]*aij[3]*aij[7];
	det -= aij[2]*aij[4]*aij[6];

	return det;
}



/**
*Returns the determinant of a complex 3x3 matrix
*@param aij a pointer to a one-dimensional array holding the 9 complex values of the matrix
*@return the matrix's determinant
*/
complex<double> det33(complex<double> *aij)
{
	//returns the determinant of a 3x3 matrix
	complex<double> det;

	det  = aij[0]*aij[4]*aij[8];
	det -= aij[0]*aij[5]*aij[7];

	det -= aij[1]*aij[3]*aij[8];
	det += aij[1]*aij[5]*aij[6];

	det += aij[2]*aij[3]*aij[7];
	det -= aij[2]*aij[4]*aij[6];

	return det;
}


/**
*Returns the determinant of a 4x4 matrix
*@param aij a pointer to a one-dimensional array holding the 16 double values of the matrix
*@return the matrix's determinant
*/
double det44(double *aij)
{
//	returns the determinant of a 4x4 matrix

	int i,j,k,l,p,q;
	double det, sign, a33[16];

	det = 0.0;
	for(i=0; i<4; i++)
	{
		for(j=0; j<4; j++)
		{
			p = 0;
			for(k=0; k<4 && k!=i; k++)
			{
				q = 0;
				for(l=0; l<4 && l!=j; l++)
				{
					*(a33+p*3+q) = *(aij+4*k+l);// could also do it by address, to be a little faster
					q++;
				}
				p++;
			}
			sign = pow(-1.0,i+j);
			det += sign * det33(a33);
		}
	}
	return det;
}



/**
*Returns the cofactor of an element in a 4x4 matrix of complex values.
*@param aij a pointer to a one-dimensional array holding the 16 complex values of the matrix.
*@param i the number of the element's line, starting at 0.
*@param j the number of the element's column, starting at 0.
*@return the cofactor of element (i,j).
*/
complex<double> cofactor44(complex<double> *aij, int &i, int &j)
{
	//returns the complex cofactor	of element i,j, in the 4x4 matrix aij
	int k,l,p,q;
	complex<double> a33[9];

	p = 0;
	for(k=0; k<4; k++)
	{
		if(k!=i)
		{
			q = 0;
			for(l=0; l<4; l++)
			{
				if(l!=j)
				{
					a33[p*3+q] = *(aij+4*k+l);
					q++;
				}
			}
			p++;
		}
	}
	return det33(a33);
}

/**
*Returns the determinant of a complex 4x4 matrix
*@param aij a pointer to a one-dimensional array holding the 16 complex double values of the matrix
*@return the matrix's determinant
*/
complex<double> det44(complex<double> *aij)
{
//	returns the determinant of a 4x4 matrix

	int i,j,k,l,p,q;
	double sign;
	complex<double> det, a33[16];
	det = 0.0;

	i=0;
	for(j=0; j<4; j++)
	{
		p = 0;
		for(k=0; k<4; k++)
		{
			if(k!=i)
			{
				q = 0;
				for(l=0; l<4; l++)
				{
					if(l!=j)
					{
						a33[p*3+q] = aij[4*k+l];
						q++;
					}
				}
				p++;
			}
		}
		sign = pow(-1.0,i+j);
		det += sign * aij[4*i+j] * det33(a33);
	}

	return det;
}







/**
  int Crout_LU_Decomposition_with_Pivoting(double *A, int pivot[], int n)

  Unknown author http:mymathlib.webtrellis.net/index.html

  Description:
	 This routine uses Crout's method to decompose a row interchanged
	 version of the n x n matrix A into a lower triangular matrix L and a
	 unit upper triangular matrix U such that A = LU.
	 The matrices L and U replace the matrix A so that the original matrix
	 A is destroyed.
	 Note!  In Crout's method the diagonal elements of U are 1 and are
			not stored.
	 Note!  The determinant of A is the product of the diagonal elements
			of L.  (det A = det L * det U = det L).
	 The LU decomposition is convenient when one needs to solve the linear
	 equation Ax = B for the vector x while the matrix A is fixed and the
	 vector B is varied.  The routine for solving the linear system Ax = B
	 after performing the LU decomposition for A is
					  Crout_LU_with_Pivoting_Solve.
	 (see below).

	 The Crout method with partial pivoting is: Determine the pivot row and
	 interchange the current row with the pivot row, then assuming that
	 row k is the current row, k = 0, ..., n - 1 evaluate in order the
	 the following pair of expressions
	   L[i][k] = (A[i][k] - (L[i][0]*U[0][k] + . + L[i][k-1]*U[k-1][k]))
								 for i = k, ... , n-1,
	   U[k][j] = A[k][j] - (L[k][0]*U[0][j] + ... + L[k][k-1]*U[k-1][j])
																  / L[k][k]
									  for j = k+1, ... , n-1.
	   The matrix U forms the upper triangular matrix, and the matrix L
	   forms the lower triangular matrix.

  Arguments:
	 double *A       Pointer to the first element of the matrix A[n][n].
	 int    pivot[]  The i-th element is the pivot row interchanged with
					 row i.
	 int     n       The number of rows or columns of the matrix A.

  Return Values:
	 0  Success
	-1  Failure - The matrix A is singular.

*/
bool Crout_LU_Decomposition_with_Pivoting(double *A, int pivot[], int n, bool *pbCancel, double TaskSize, double &Progress)
{
	int i, j, k;
	double *p_k, *p_row, *p_col;
	double max=0.0;

	p_col = NULL;

	//  For each row and column, k = 0, ..., n-1,
	for (k=0, p_k=A; k<n; p_k+=n, k++)
	{
	//  find the pivot row
		pivot[k] = k;
		p_col = p_k+k;
		max = qAbs( *(p_k + k) );
		for (j=k+1, p_row=p_k+n; j<n; j++, p_row+=n)
		{
			if (max<qAbs(*(p_row+k)))
			{
				max = qAbs(*(p_row+k));
				pivot[k] = j;
				p_col = p_row;
			}
		}
		if(!p_col) return false;

		// and if the pivot row differs from the current row, then
		// interchange the two rows.
		if (pivot[k] != k)
		{
			for (j=0; j<n; j++)
			{
				max = *(p_k + j);
				*(p_k + j) = *(p_col + j);
				*(p_col + j) = max;
			}
		}

		// and if the matrix is singular, return error
		if ( *(p_k + k) == 0.0 ) return false;

		// otherwise find the upper triangular matrix elements for row k.
		for (j = k+1; j < n; j++) *(p_k + j) /= *(p_k + k);

		// update remaining matrix
		for (i = k+1, p_row = p_k + n; i < n; p_row += n, i++)
			for (j = k+1; j < n; j++) *(p_row + j) -= *(p_row + k) * *(p_k + j);

		Progress += TaskSize/(double)(n);
//		qApp->processEvents();
		if(*pbCancel) return false;
	}
	return true;
}

bool Crout_LU_Decomposition_with_Pivoting(float *A, int pivot[], int n, bool *pbCancel, double TaskSize, double &Progress)
{
	int i, j, k;
	float *p_k, *p_row, *p_col;
	float max=0.0;

	p_col = NULL;

	//  For each row and column, k = 0, ..., n-1,
	for (k=0, p_k=A; k<n; p_k+=n, k++)
	{
	//  find the pivot row
		pivot[k] = k;
		p_col = p_k+k;
		max = qAbs( *(p_k + k) );
		for (j=k+1, p_row=p_k+n; j<n; j++, p_row+=n)
		{
			if (max<qAbs(*(p_row+k)))
			{
				max = qAbs(*(p_row+k));
				pivot[k] = j;
				p_col = p_row;
			}
		}
		if(!p_col) return false;

		// and if the pivot row differs from the current row, then
		// interchange the two rows.
		if (pivot[k] != k)
		{
			for (j=0; j<n; j++)
			{
				max = *(p_k + j);
				*(p_k + j) = *(p_col + j);
				*(p_col + j) = max;
			}
		}

		// and if the matrix is singular, return error
		if ( *(p_k + k) == 0.0 ) return false;

		// otherwise find the upper triangular matrix elements for row k.
		for (j = k+1; j < n; j++) *(p_k + j) /= *(p_k + k);

		// update remaining matrix
		for (i = k+1, p_row = p_k + n; i < n; p_row += n, i++)
			for (j = k+1; j < n; j++) *(p_row + j) -= *(p_row + k) * *(p_k + j);

		Progress += TaskSize/(double)(n);
//		qApp->processEvents();
		if(*pbCancel) return false;
	}
	return true;
}


/**
  int Crout_LU_with_Pivoting_Solve(double *LU, double B[], int pivot[],
														double x[], int n)

 Unknown author http:mymathlib.webtrellis.net/index.html

  Description:
	 This routine uses Crout's method to solve the linear equation Ax = B.
	 This routine is called after the matrix A has been decomposed into a
	 product of a lower triangular matrix L and a unit upper triangular
	 matrix U without pivoting.  The argument LU is a pointer to the matrix
	 the superdiagonal part of which is U and the subdiagonal together with
	 the diagonal part is L. (The diagonal part of U is 1 and is not
	 stored.)   The matrix A = LU.
	 The solution proceeds by solving the linear equation Ly = B for y and
	 subsequently solving the linear equation Ux = y for x.

  Arguments:
	 double *LU      Pointer to the first element of the matrix whose
					 elements form the lower and upper triangular matrix
					 factors of A.
	 double *B       Pointer to the column vector, (n x 1) matrix, B.
	 int    pivot[]  The i-th element is the pivot row interchanged with
					 row i.
	 double *x       Solution to the equation Ax = B.
	 int     n       The number of rows or columns of the matrix LU.

  Return Values:
	 true  : Success
	 false : Failure - The matrix A is singular.

*/
bool Crout_LU_with_Pivoting_Solve(double *LU, double B[], int pivot[], double x[], int Size, bool *pbCancel)
{
	int i, k;
	double *p_k;
	double dum;

	//  Solve the linear equation Lx = B for x, where L is a lower triangular matrix.
	for (k=0, p_k=LU; k<Size; p_k+=Size, k++)
	{
		if (pivot[k] != k)
		{
			dum=B[k]; B[k]=B[pivot[k]]; B[pivot[k]]=dum;
		}

		x[k] = B[k];
		for (i=0; i<k; i++) x[k]-=x[i] * *(p_k+i);
		x[k] /= *(p_k+k);

//		qApp->processEvents();
		if(*pbCancel) return false;
	}

	//  Solve the linear equation Ux = y, where y is the solution
	//  obtained above of Lx = B and U is an upper triangular matrix.
	//  The diagonal part of the upper triangular part of the matrix is
	//  assumed to be 1.0.
	for (k=Size-1, p_k=LU+Size*(Size-1); k>=0; k--, p_k-=Size)
	{
		if (pivot[k] != k)
		{
			dum=B[k]; B[k]=B[pivot[k]]; B[pivot[k]]=dum;
		}

		for (i=k+1; i<Size; i++) x[k]-=x[i] * *(p_k+i);
		if (*(p_k+k)==0.0)
		{
			return false;
		}

//		qApp->processEvents();
		if(*pbCancel) return false;
	}

	return true;
}

bool Crout_LU_with_Pivoting_Solve(float *matLU, double B[], int pivot[], double x[], int Size, bool *pbCancel)
{
	int i, k;
	float *pk;
	double dum;

	//  Solve the linear equation Lx = B for x, where L is a lower triangular matrix.
	for (k=0, pk=matLU; k<Size; pk+=Size, k++)
	{
		if (pivot[k] != k)
		{
			dum=B[k]; B[k]=B[pivot[k]]; B[pivot[k]]=dum;
		}

		x[k] = B[k];
		for (i=0; i<k; i++) x[k]-=x[i] * *(pk+i);
		x[k] /= *(pk+k);

//		qApp->processEvents();
		if(*pbCancel) return false;
	}

	//  Solve the linear equation Ux = y, where y is the solution
	//  obtained above of Lx = B and U is an upper triangular matrix.
	//  The diagonal part of the upper triangular part of the matrix is
	//  assumed to be 1.0.
	for (k=Size-1, pk=matLU+Size*(Size-1); k>=0; k--, pk-=Size)
	{
		if (pivot[k] != k)
		{
			dum=B[k]; B[k]=B[pivot[k]]; B[pivot[k]]=dum;
		}

		for (i=k+1; i<Size; i++) x[k]-=x[i] * *(pk+i);
		if (fabs(*(pk+k))<PRECISION)
		{
			return false;
		}

//		qApp->processEvents();
		if(*pbCancel) return false;
	}

	return true;
}


/**
* Returns the coefficients of the characteristic polynomial of a 4x4 matrix of double values. Thanks Mapple.
* The polynom can then be solved for complex roots using Bairstow's algorithm
*@param m the 4x4 matrix
*@param p the array holding the 5 coefficients of the matrix characteristic polynomial
*/
void CharacteristicPol(double m[][4], double p[5])
{
	// lambda^4
	p[4] =  1;

	// lambda^3
	p[3] =  - m[0][0]- m[1][1]-m[2][2]- m[3][3];

	// lambda^2
	p[2] =
		+ m[0][0] * m[1][1]
		+ m[0][0] * m[2][2]
		+ m[0][0] * m[3][3]
		+ m[1][1] * m[3][3]
		+ m[1][1] * m[2][2]
		+ m[2][2] * m[3][3]
		- m[1][0] * m[0][1]
		- m[2][1] * m[1][2]
		- m[2][0] * m[0][2]
		- m[2][3] * m[3][2]
		- m[3][1] * m[1][3]
		- m[3][0] * m[0][3];

	// lambda^1
	p[1] =
		+ m[2][1] * m[1][2] * m[3][3]
		+ m[0][0] * m[2][1] * m[1][2]
		- m[3][1] * m[1][2] * m[2][3]
		+ m[3][1] * m[1][3] * m[2][2]
		+ m[1][0] * m[0][1] * m[3][3]
		+ m[1][0] * m[0][1] * m[2][2]
		- m[2][0] * m[0][1] * m[1][2]
		- m[1][0] * m[3][1] * m[0][3]
		- m[1][0] * m[2][1] * m[0][2]
		+ m[3][0] * m[1][1] * m[0][3]
		- m[3][0] * m[0][1] * m[1][3]
		+ m[2][0] * m[0][2] * m[3][3]
		- m[2][0] * m[3][2] * m[0][3]
		+ m[2][0] * m[1][1] * m[0][2]
		- m[3][0] * m[0][2] * m[2][3]
		+ m[3][0] * m[0][3] * m[2][2]
		- m[2][1] * m[3][2] * m[1][3]
		- m[0][0] * m[1][1] * m[2][2]
		+ m[0][0] * m[2][3] * m[3][2]
		+ m[1][1] * m[2][3] * m[3][2]
		- m[0][0] * m[2][2] * m[3][3]
		+ m[0][0] * m[3][1] * m[1][3]
		- m[1][1] * m[2][2] * m[3][3]
		- m[0][0] * m[1][1] * m[3][3];

	// lambda^0
	p[0] =
		+ m[2][0] * m[0][1] * m[1][2] * m[3][3]
		- m[2][0] * m[1][1] * m[0][2] * m[3][3]
		+ m[2][0] * m[1][1] * m[3][2] * m[0][3]
		- m[2][0] * m[0][1] * m[3][2] * m[1][3]
		+ m[1][0] * m[3][1] * m[0][3] * m[2][2]
		- m[1][0] * m[3][1] * m[0][2] * m[2][3]
		+ m[1][0] * m[2][1] * m[0][2] * m[3][3]
		- m[1][0] * m[2][1] * m[3][2] * m[0][3]
		- m[3][0] * m[1][1] * m[0][3] * m[2][2]
		+ m[3][0] * m[0][1] * m[1][3] * m[2][2]
		- m[3][0] * m[0][1] * m[1][2] * m[2][3]
		- m[2][0] * m[3][1] * m[1][2] * m[0][3]
		+ m[2][0] * m[3][1] * m[0][2] * m[1][3]
		- m[3][0] * m[2][1] * m[0][2] * m[1][3]
		+ m[3][0] * m[1][1] * m[0][2] * m[2][3]
		+ m[3][0] * m[2][1] * m[1][2] * m[0][3]
		- m[0][0] * m[2][1] * m[1][2] * m[3][3]
		+ m[0][0] * m[2][1] * m[3][2] * m[1][3]
		- m[1][0] * m[0][1] * m[2][2] * m[3][3]
		+ m[1][0] * m[0][1] * m[2][3] * m[3][2]
		+ m[0][0] * m[3][1] * m[1][2] * m[2][3]
		- m[0][0] * m[3][1] * m[1][3] * m[2][2]
		+ m[0][0] * m[1][1] * m[2][2] * m[3][3]
		- m[0][0] * m[1][1] * m[2][3] * m[3][2];
}


#define POLYNOMORDER    6


/** Text function, for debugging purposes only*/
void TestEigen()
{
	double A[4][4];
	double p[5];
/*	A[0][0] = -0.0069; 	A[1][0] =  0.0139; 	A[2][0] =   0.0;    A[3][0] = -9.81;
	A[0][1] = -0.0905; 	A[1][1] = -0.3149; 	A[2][1] = 235.8928; A[3][1] =  0.0;
	A[0][2] =  0.0004;	A[1][2] = -0.0034;	A[2][2] = -0.4282;  A[3][2] = 0.0;
	A[0][3] =  0.0000;	A[1][3] =  0.0000;	A[2][3] =  1.0;     A[3][3] = 0.0;*/

	A[0][0] =-1.00; A[0][1] =  1.0;	A[0][2] =  1.0;	A[0][3] = -1.0;
	A[1][0] = 1.00; A[1][1] =  1.0;	A[1][2] =  2.0; A[1][3] = -1.0;
	A[2][0] = 3.00;	A[2][1] = -2.0;	A[2][2] =  1.0; A[2][3] =  1.0;
	A[3][0] = 1.00;	A[3][1] =  1.0;	A[3][2] =  2.0;	A[3][3] =  1.0;
	complex<double> AC[16];
//	complex<double> V[4];
	complex <double> lambda(2.0, 0.0);
	for(int i=0; i<4; i++)
	{
		for(int j=0; j<4;j++)
		{
			AC[i*4+j] = complex<double>(A[i][j],0.0);
		}
	}

	CharacteristicPol(A, p);

	complex<double> roots[POLYNOMORDER];

	if(LinBairstow(p, roots, 4))
	{
	}
	else
	{
	}
}


/**________________________________________________________________________
* Finds the eigenvector associated to an eigenvalue.
* Solves the system A.V = lambda.V where A is a 4x4 complex matrix
* in input :
*    - matrix A
*    - the array of complex eigenvalues
* in output
*    - the array of complex eigenvectors
*
* The eigenvector is calculated by direct matrix inversion.
* One of the vector's component is set to 1, to avoid the trivial solution V=0;
*
* (c) Andre Deperrois October 2009
*@param a the complex two-dimensional 4x4 input matrix to diagonalize
*@param lambda the output array of four complex eigenvalues
*@param V the eigenvector as a one-dimensional array of complex values
*________________________________________________________________________ */
bool Eigenvector(double a[][4], complex<double> lambda, complex<double> *V)
{
	complex<double> detm, detr;
	complex<double> r[9], m[9];
	int ii, jj, i, j, kp;

	// first find a pivot for which the  associated n-1 determinant is not zero
	bool bFound = false;
	kp=0;
	do
	{
		V[kp] = 1.0;
		ii= 0;
		for(i=0;i<4 ; i++)
		{
			if(i!=kp)
			{
				jj=0;
				for(j=0; j<4; j++)
				{
					if(j!=kp)
					{
						m[ii*3+jj] = a[i][j];
						jj++;
					}
				}
				m[ii*3+ii] -= lambda;
				ii++;
			}
		}
		detm = det33(m);
		bFound = std::abs(detm)>0.0;
		if(bFound || kp>=3) break;
		kp++;
	}while(true);

	if(!bFound) return false;

	// at this point we have identified pivot kp
	// with a non-zero subdeterminant.
	// so solve for the other 3 eigenvector components.
	// using Cramer's rule

	//create rhs determinant
	jj=0;
	for(j=0; j<4; j++)
	{
		memcpy(r,m, 9*sizeof(complex<double>));
		if(j!=kp)
		{
			ii= 0;
			for(i=0; i<4; i++)
			{
				if(i!=kp)
				{
					r[ii*3+jj] = - a[i][kp];
					ii++;
				}
			}
			detr  = det33(r);
			V[j] = detr/detm;
			jj++;
		}
	}

	return true;
}


#define TOLERANCE   1.e-8
#define MAXBAIRSTOWITER 100

/**
* Finds the complex roots of a polynom P(x) using Lin-Bairstow's method
* P(x) = Sum p_i x^i	    i = 0..n;
* The polynoms coefficient are in array p
*
* Andre Deperrois October 2009
*@param p the array of the polynoms double's coefficients
*@param root the array of the polynom's complex roots
*@param n the polynom's order
*@return true if the extraction was successful
*/
bool LinBairstow(double *p, complex<double> *root, int n)
{
	double b[POLYNOMORDER], c[POLYNOMORDER];
	int i, k, nn, iter;
	double r,s,d0,d1,d2;
	double Delta;

	memset(b, 0, POLYNOMORDER*sizeof(double));
	memset(c, 0, POLYNOMORDER*sizeof(double));

	//T(x) = x2 -rx -s;
	//R(x) = u(x-r)+v //remainder of deivision of by Q
	//Q(x) = Sum b_i x^(i-2)

	//P(x) = (x2-rx+s) Q(x) + u(x-r) +v

	nn=n ;//initial order is polynom order

	r=-2.0;//initial guesses
	s=-1.0;

	do
	{
		iter = 0;
		do
		{
			//compute recursively the coefs b_i of polynom Q
			b[nn]   = p[nn];
			b[nn-1] = p[nn-1] + r * b[nn];
			for(k=nn-2; k>=0; k--) b[k] = p[k] + r*b[k+1] + s*b[k+2];

			//build the partial derivatives c_i
			c[nn]   = b[nn];
			c[nn-1] = b[nn-1] + r * c[nn];
			for(k=nn-2; k>=1; k--) c[k] = b[k] + r*c[k+1] + s*c[k+2];

			d0 = c[1]*c[3] - c[2]*c[2];
			d1 = (-b[0]*c[3]+b[1]*c[2])/d0;
			d2 = (-b[1]*c[1]+b[0]*c[2])/d0;
			r+=d1;
			s+=d2;
			iter++;
		} while((std::abs(d1)> TOLERANCE || std::abs(d2)> TOLERANCE) && iter < MAXBAIRSTOWITER);

		if(iter>=MAXBAIRSTOWITER)return false;
		//we have a division
		//so find the roots of the remainder R
		Delta = r*r+4.0*s;
		if(Delta<0.0)
		{
			//complex roots
			root[nn-1] = complex<double>(r/2.0,  sqrt(qAbs(Delta))/2.0);
			root[nn-2] = complex<double>(r/2.0, -sqrt(qAbs(Delta))/2.0);
		}
		else
		{
			//real roots
			root[nn-1] = complex<double>(r/2.0 + sqrt(Delta)/2.0, 0.0);
			root[nn-2] = complex<double>(r/2.0 - sqrt(Delta)/2.0, 0.0);
		}

		//deflate polynom order
		for(i=nn; i>=2; i--)
		{
			p[i-2] = b[i];
		}
		nn-=2;
		if(nn==2)
		{
			//last two roots, solve directly
			if(qAbs(p[2])<PRECISION)
			{
				// one last root, but we should never get here
				if(qAbs(p[1])>PRECISION)
				{
					//last single root, real
					root[0] = -p[0]/p[1];
				}
				else return false;
			}
			else
			{
				Delta = p[1]*p[1]-4.0*p[0]*p[2];
				if(Delta<0)
				{
					//complex roots
					root[nn-1] = complex<double>(-p[1]/2.0/p[2],  sqrt(qAbs(Delta))/2.0/p[2]);
					root[nn-2] = complex<double>(-p[1]/2.0/p[2], -sqrt(qAbs(Delta))/2.0/p[2]);
				}
				else
				{
					//real roots
					root[nn-1] = complex<double>((-p[1]+sqrt(Delta))/2.0/p[2],  0.0);
					root[nn-2] = complex<double>((-p[1]-sqrt(Delta))/2.0/p[2],  0.0);
				}
			}
			break;
		}
		if(nn==1)
		{
			if(qAbs(p[1])>PRECISION)
			{
				//last single root, real
				root[0] = -p[0]/p[1];
			}
			else return false;
			break;
		}

	}while(nn>2);
	return true;
}

