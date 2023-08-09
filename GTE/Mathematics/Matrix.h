// David Eberly, Geometric Tools, Redmond WA 98052
// Copyright (c) 1998-2023
// Distributed under the Boost Software License, Version 1.0.
// https://www.boost.org/LICENSE_1_0.txt
// https://www.geometrictools.com/License/Boost/LICENSE_1_0.txt
// Version: 6.0.2023.08.08

#pragma once

#include <Mathematics/Vector.h>
#include <Mathematics/GaussianElimination.h>
#include <array>
#include <cmath>
#include <cstddef>
#include <cstdint>
#include <initializer_list>

namespace gte
{
    template <int32_t NumRows, int32_t NumCols, typename Real>
    class Matrix
    {
    public:
        // The table is initialized to zero.
        Matrix()
        {
            MakeZero();
        }

        // The table is fully initialized by the inputs.  The 'values' must be
        // specified in row-major order, regardless of the active storage
        // scheme (GTE_USE_ROW_MAJOR or GTE_USE_COL_MAJOR).
        Matrix(std::array<Real, NumRows * NumCols> const& values)
        {
            for (int32_t r = 0, i = 0; r < NumRows; ++r)
            {
                for (int32_t c = 0; c < NumCols; ++c, ++i)
                {
                    mTable(r, c) = values[i];
                }
            }
        }

        // At most NumRows*NumCols are copied from the initializer list,
        // setting any remaining elements to zero.  The 'values' must be
        // specified in row-major order, regardless of the active storage
        // scheme (GTE_USE_ROW_MAJOR or GTE_USE_COL_MAJOR).  Create the zero
        // matrix using the syntax
        //   Matrix<NumRows,NumCols,Real> zero{(Real)0};
        // WARNING: The C++ 11 specification states that
        //   Matrix<NumRows,NumCols,Real> zero{};
        // will lead to a call of the default constructor, not the initializer
        // constructor!
        Matrix(std::initializer_list<Real> values)
        {
            int32_t const numValues = static_cast<int32_t>(values.size());
            auto iter = values.begin();
            int32_t r, c, i;
            for (r = 0, i = 0; r < NumRows; ++r)
            {
                for (c = 0; c < NumCols; ++c, ++i)
                {
                    if (i < numValues)
                    {
                        mTable(r, c) = *iter++;
                    }
                    else
                    {
                        break;
                    }
                }

                if (c < NumCols)
                {
                    // Fill in the remaining columns of the current row with zeros.
                    for (/**/; c < NumCols; ++c)
                    {
                        mTable(r, c) = (Real)0;
                    }
                    ++r;
                    break;
                }
            }

            if (r < NumRows)
            {
                // Fill in the remain rows with zeros.
                for (/**/; r < NumRows; ++r)
                {
                    for (c = 0; c < NumCols; ++c)
                    {
                        mTable(r, c) = (Real)0;
                    }
                }
            }
        }

        // For 0 <= r < NumRows and 0 <= c < NumCols, element (r,c) is 1 and
        // all others are 0.  If either of r or c is invalid, the zero matrix
        // is created.  This is a convenience for creating the standard
        // Euclidean basis matrices; see also MakeUnit(int32_t,int32_t) and
        // Unit(int32_t,int32_t).
        Matrix(int32_t r, int32_t c)
        {
            MakeUnit(r, c);
        }

        // The copy constructor, destructor, and assignment operator are
        // generated by the compiler.

        // Member access for which the storage representation is transparent.
        // The matrix entry in row r and column c is A(r,c).  The first
        // operator() returns a const reference rather than a Real value.
        // This supports writing via standard file operations that require a
        // const pointer to data.
        inline Real const& operator()(int32_t r, int32_t c) const
        {
            return mTable(r, c);
        }

        inline Real& operator()(int32_t r, int32_t c)
        {
            return mTable(r, c);
        }

        // Member access by rows or by columns.
        void SetRow(int32_t r, Vector<NumCols, Real> const& vec)
        {
            for (int32_t c = 0; c < NumCols; ++c)
            {
                mTable(r, c) = vec[c];
            }
        }

        void SetCol(int32_t c, Vector<NumRows, Real> const& vec)
        {
            for (int32_t r = 0; r < NumRows; ++r)
            {
                mTable(r, c) = vec[r];
            }
        }

        Vector<NumCols, Real> GetRow(int32_t r) const
        {
            Vector<NumCols, Real> vec;
            for (int32_t c = 0; c < NumCols; ++c)
            {
                vec[c] = mTable(r, c);
            }
            return vec;
        }

        Vector<NumRows, Real> GetCol(int32_t c) const
        {
            Vector<NumRows, Real> vec;
            for (int32_t r = 0; r < NumRows; ++r)
            {
                vec[r] = mTable(r, c);
            }
            return vec;
        }

        // Member access by 1-dimensional index.  NOTE: These accessors are
        // useful for the manipulation of matrix entries when it does not
        // matter whether storage is row-major or column-major.  Do not use
        // constructs such as M[c+NumCols*r] or M[r+NumRows*c] that expose the
        // storage convention.
        inline Real const& operator[](int32_t i) const
        {
            return mTable[i];
        }

        inline Real& operator[](int32_t i)
        {
            return mTable[i];
        }

        // Comparisons for sorted containers and geometric ordering.
        inline bool operator==(Matrix const& mat) const
        {
            return mTable.mStorage == mat.mTable.mStorage;
        }

        inline bool operator!=(Matrix const& mat) const
        {
            return mTable.mStorage != mat.mTable.mStorage;
        }

        inline bool operator< (Matrix const& mat) const
        {
            return mTable.mStorage < mat.mTable.mStorage;
        }

        inline bool operator<=(Matrix const& mat) const
        {
            return mTable.mStorage <= mat.mTable.mStorage;
        }

        inline bool operator> (Matrix const& mat) const
        {
            return mTable.mStorage > mat.mTable.mStorage;
        }

        inline bool operator>=(Matrix const& mat) const
        {
            return mTable.mStorage >= mat.mTable.mStorage;
        }

        // Special matrices.

        // All components are 0.
        void MakeZero()
        {
            Real const zero(0);
            for (int32_t i = 0; i < NumRows * NumCols; ++i)
            {
                mTable[i] = zero;
            }
        }

        // Component (r,c) is 1, all others zero.
        void MakeUnit(int32_t r, int32_t c)
        {
            MakeZero();
            if (0 <= r && r < NumRows && 0 <= c && c < NumCols)
            {
                mTable(r, c) = (Real)1;
            }
        }

        // Diagonal entries 1, others 0, even when nonsquare
        void MakeIdentity()
        {
            MakeZero();
            int32_t const numDiagonal = (NumRows <= NumCols ? NumRows : NumCols);
            for (int32_t i = 0; i < numDiagonal; ++i)
            {
                mTable(i, i) = (Real)1;
            }
        }

        static Matrix Zero()
        {
            Matrix M;
            M.MakeZero();
            return M;
        }

        static Matrix Unit(int32_t r, int32_t c)
        {
            Matrix M;
            M.MakeUnit(r, c);
            return M;
        }

        static Matrix Identity()
        {
            Matrix M;
            M.MakeIdentity();
            return M;
        }

    protected:
        class Table
        {
        public:
            Table()
                :
                mStorage{}
            {
#if defined(GTE_USE_ROW_MAJOR)
                for (size_t r = 0; r < NumRows; ++r)
                {
                    for (size_t c = 0; c < NumCols; ++c)
                    {
                        mStorage[r][c] = (Real)0;
                    }
                }
#else
                for (size_t c = 0; c < NumCols; ++c)
                {
                    for (size_t r = 0; r < NumRows; ++r)
                    {
                        mStorage[c][r] = (Real)0;
                    }
                }
#endif
            }

            // Storage-order-independent element access as 2D array.
            inline Real const& operator()(int32_t r, int32_t c) const
            {
#if defined(GTE_USE_ROW_MAJOR)
                return mStorage[r][c];
#else
                return mStorage[c][r];
#endif
            }

            inline Real& operator()(int32_t r, int32_t c)
            {
#if defined(GTE_USE_ROW_MAJOR)
                return mStorage[r][c];
#else
                return mStorage[c][r];
#endif
            }

            // Element access as 1D array.  Use this internally only when
            // the 2D storage order is not relevant.
            inline Real const& operator[](int32_t i) const
            {
                Real const* elements = &mStorage[0][0];
                return elements[i];
            }

            inline Real& operator[](int32_t i)
            {
                Real* elements = &mStorage[0][0];
                return elements[i];
            }

#if defined(GTE_USE_ROW_MAJOR)
            std::array<std::array<Real, NumCols>, NumRows> mStorage;
#else
            std::array<std::array<Real, NumRows>, NumCols> mStorage;
#endif
        };

        Table mTable;
    };

    // Unary operations.
    template <int32_t NumRows, int32_t NumCols, typename Real>
    Matrix<NumRows, NumCols, Real> operator+(Matrix<NumRows, NumCols, Real> const& M)
    {
        return M;
    }

    template <int32_t NumRows, int32_t NumCols, typename Real>
    Matrix<NumRows, NumCols, Real> operator-(Matrix<NumRows, NumCols, Real> const& M)
    {
        Matrix<NumRows, NumCols, Real> result;
        for (int32_t i = 0; i < NumRows * NumCols; ++i)
        {
            result[i] = -M[i];
        }
        return result;
    }

    // Linear-algebraic operations.
    template <int32_t NumRows, int32_t NumCols, typename Real>
    Matrix<NumRows, NumCols, Real> operator+(
        Matrix<NumRows, NumCols, Real> const& M0,
        Matrix<NumRows, NumCols, Real> const& M1)
    {
        Matrix<NumRows, NumCols, Real> result = M0;
        return result += M1;
    }

    template <int32_t NumRows, int32_t NumCols, typename Real>
    Matrix<NumRows, NumCols, Real> operator-(
        Matrix<NumRows, NumCols, Real> const& M0,
        Matrix<NumRows, NumCols, Real> const& M1)
    {
        Matrix<NumRows, NumCols, Real> result = M0;
        return result -= M1;
    }

    template <int32_t NumRows, int32_t NumCols, typename Real>
    Matrix<NumRows, NumCols, Real> operator*(
        Matrix<NumRows, NumCols, Real> const& M, Real scalar)
    {
        Matrix<NumRows, NumCols, Real> result = M;
        return result *= scalar;
    }

    template <int32_t NumRows, int32_t NumCols, typename Real>
    Matrix<NumRows, NumCols, Real> operator*(
        Real scalar, Matrix<NumRows, NumCols, Real> const& M)
    {
        Matrix<NumRows, NumCols, Real> result = M;
        return result *= scalar;
    }

    template <int32_t NumRows, int32_t NumCols, typename Real>
    Matrix<NumRows, NumCols, Real> operator/(
        Matrix<NumRows, NumCols, Real> const& M, Real scalar)
    {
        Matrix<NumRows, NumCols, Real> result = M;
        return result /= scalar;
    }

    template <int32_t NumRows, int32_t NumCols, typename Real>
    Matrix<NumRows, NumCols, Real>& operator+=(
        Matrix<NumRows, NumCols, Real>& M0,
        Matrix<NumRows, NumCols, Real> const& M1)
    {
        for (int32_t i = 0; i < NumRows * NumCols; ++i)
        {
            M0[i] += M1[i];
        }
        return M0;
    }

    template <int32_t NumRows, int32_t NumCols, typename Real>
    Matrix<NumRows, NumCols, Real>& operator-=(
        Matrix<NumRows, NumCols, Real>& M0,
        Matrix<NumRows, NumCols, Real> const& M1)
    {
        for (int32_t i = 0; i < NumRows * NumCols; ++i)
        {
            M0[i] -= M1[i];
        }
        return M0;
    }

    template <int32_t NumRows, int32_t NumCols, typename Real>
    Matrix<NumRows, NumCols, Real>& operator*=(
        Matrix<NumRows, NumCols, Real>& M, Real scalar)
    {
        for (int32_t i = 0; i < NumRows * NumCols; ++i)
        {
            M[i] *= scalar;
        }
        return M;
    }

    template <int32_t NumRows, int32_t NumCols, typename Real>
    Matrix<NumRows, NumCols, Real>& operator/=(
        Matrix<NumRows, NumCols, Real>& M, Real scalar)
    {
        if (scalar != (Real)0)
        {
            Real invScalar = ((Real)1) / scalar;
            for (int32_t i = 0; i < NumRows * NumCols; ++i)
            {
                M[i] *= invScalar;
            }
        }
        else
        {
            for (int32_t i = 0; i < NumRows * NumCols; ++i)
            {
                M[i] = (Real)0;
            }
        }
        return M;
    }

    // Geometric operations.
    template <int32_t NumRows, int32_t NumCols, typename Real>
    Real L1Norm(Matrix<NumRows, NumCols, Real> const& M)
    {
        Real sum = std::fabs(M[0]);
        for (int32_t i = 1; i < NumRows * NumCols; ++i)
        {
            sum += std::fabs(M[i]);
        }
        return sum;
    }

    template <int32_t NumRows, int32_t NumCols, typename Real>
    Real L2Norm(Matrix<NumRows, NumCols, Real> const& M)
    {
        Real sum = M[0] * M[0];
        for (int32_t i = 1; i < NumRows * NumCols; ++i)
        {
            sum += M[i] * M[i];
        }
        return std::sqrt(sum);
    }

    template <int32_t NumRows, int32_t NumCols, typename Real>
    Real LInfinityNorm(Matrix<NumRows, NumCols, Real> const& M)
    {
        Real maxAbsElement = M[0];
        for (int32_t i = 1; i < NumRows * NumCols; ++i)
        {
            Real absElement = std::fabs(M[i]);
            if (absElement > maxAbsElement)
            {
                maxAbsElement = absElement;
            }
        }
        return maxAbsElement;
    }

    template <int32_t N, typename Real>
    Matrix<N, N, Real> Inverse(Matrix<N, N, Real> const& M, bool* reportInvertibility = nullptr)
    {
        Matrix<N, N, Real> invM;
        Real determinant{};
        bool invertible = GaussianElimination<Real>()(N, &M[0], &invM[0],
            determinant, nullptr, nullptr, nullptr, 0, nullptr);
        if (reportInvertibility)
        {
            *reportInvertibility = invertible;
        }
        return invM;
    }

    template <int32_t N, typename Real>
    Real Determinant(Matrix<N, N, Real> const& M)
    {
        Real determinant{};
        GaussianElimination<Real>()(N, &M[0], nullptr, determinant, nullptr,
            nullptr, nullptr, 0, nullptr);
        return determinant;
    }

    // M^T
    template <int32_t NumRows, int32_t NumCols, typename Real>
    Matrix<NumCols, NumRows, Real> Transpose(Matrix<NumRows, NumCols, Real> const& M)
    {
        Matrix<NumCols, NumRows, Real> result;
        for (int32_t r = 0; r < NumRows; ++r)
        {
            for (int32_t c = 0; c < NumCols; ++c)
            {
                result(c, r) = M(r, c);
            }
        }
        return result;
    }

    // M*V
    template <int32_t NumRows, int32_t NumCols, typename Real>
    Vector<NumRows, Real> operator*(Matrix<NumRows, NumCols, Real> const& M,
        Vector<NumCols, Real> const& V)
    {
        Vector<NumRows, Real> result;
        for (int32_t r = 0; r < NumRows; ++r)
        {
            result[r] = (Real)0;
            for (int32_t c = 0; c < NumCols; ++c)
            {
                result[r] += M(r, c) * V[c];
            }
        }
        return result;
    }

    // V^T*M
    template <int32_t NumRows, int32_t NumCols, typename Real>
    Vector<NumCols, Real> operator*(Vector<NumRows, Real> const& V,
        Matrix<NumRows, NumCols, Real> const& M)
    {
        Vector<NumCols, Real> result;
        for (int32_t c = 0; c < NumCols; ++c)
        {
            result[c] = (Real)0;
            for (int32_t r = 0; r < NumRows; ++r)
            {
                result[c] += V[r] * M(r, c);
            }
        }
        return result;
    }

    // A*B
    template <int32_t NumRows, int32_t NumCols, int32_t NumCommon, typename Real>
    Matrix<NumRows, NumCols, Real> operator*(
        Matrix<NumRows, NumCommon, Real> const& A,
        Matrix<NumCommon, NumCols, Real> const& B)
    {
        return MultiplyAB(A, B);
    }

    template <int32_t NumRows, int32_t NumCols, int32_t NumCommon, typename Real>
    Matrix<NumRows, NumCols, Real> MultiplyAB(
        Matrix<NumRows, NumCommon, Real> const& A,
        Matrix<NumCommon, NumCols, Real> const& B)
    {
        Matrix<NumRows, NumCols, Real> result;
        for (int32_t r = 0; r < NumRows; ++r)
        {
            for (int32_t c = 0; c < NumCols; ++c)
            {
                result(r, c) = (Real)0;
                for (int32_t i = 0; i < NumCommon; ++i)
                {
                    result(r, c) += A(r, i) * B(i, c);
                }
            }
        }
        return result;
    }

    // A*B^T
    template <int32_t NumRows, int32_t NumCols, int32_t NumCommon, typename Real>
    Matrix<NumRows, NumCols, Real>  MultiplyABT(
        Matrix<NumRows, NumCommon, Real> const& A,
        Matrix<NumCols, NumCommon, Real> const& B)
    {
        Matrix<NumRows, NumCols, Real> result;
        for (int32_t r = 0; r < NumRows; ++r)
        {
            for (int32_t c = 0; c < NumCols; ++c)
            {
                result(r, c) = (Real)0;
                for (int32_t i = 0; i < NumCommon; ++i)
                {
                    result(r, c) += A(r, i) * B(c, i);
                }
            }
        }
        return result;
    }

    // A^T*B
    template <int32_t NumRows, int32_t NumCols, int32_t NumCommon, typename Real>
    Matrix<NumRows, NumCols, Real> MultiplyATB(
        Matrix<NumCommon, NumRows, Real> const& A,
        Matrix<NumCommon, NumCols, Real> const& B)
    {
        Matrix<NumRows, NumCols, Real> result;
        for (int32_t r = 0; r < NumRows; ++r)
        {
            for (int32_t c = 0; c < NumCols; ++c)
            {
                result(r, c) = (Real)0;
                for (int32_t i = 0; i < NumCommon; ++i)
                {
                    result(r, c) += A(i, r) * B(i, c);
                }
            }
        }
        return result;
    }

    // A^T*B^T
    template <int32_t NumRows, int32_t NumCols, int32_t NumCommon, typename Real>
    Matrix<NumRows, NumCols, Real> MultiplyATBT(
        Matrix<NumCommon, NumRows, Real> const& A,
        Matrix<NumCols, NumCommon, Real> const& B)
    {
        Matrix<NumRows, NumCols, Real> result;
        for (int32_t r = 0; r < NumRows; ++r)
        {
            for (int32_t c = 0; c < NumCols; ++c)
            {
                result(r, c) = (Real)0;
                for (int32_t i = 0; i < NumCommon; ++i)
                {
                    result(r, c) += A(i, r) * B(c, i);
                }
            }
        }
        return result;
    }

    // M*D, D is diagonal NumCols-by-NumCols
    template <int32_t NumRows, int32_t NumCols, typename Real>
    Matrix<NumRows, NumCols, Real> MultiplyMD(
        Matrix<NumRows, NumCols, Real> const& M,
        Vector<NumCols, Real> const& D)
    {
        Matrix<NumRows, NumCols, Real> result;
        for (int32_t r = 0; r < NumRows; ++r)
        {
            for (int32_t c = 0; c < NumCols; ++c)
            {
                result(r, c) = M(r, c) * D[c];
            }
        }
        return result;
    }

    // D*M, D is diagonal NumRows-by-NumRows
    template <int32_t NumRows, int32_t NumCols, typename Real>
    Matrix<NumRows, NumCols, Real>  MultiplyDM(
        Vector<NumRows, Real> const& D,
        Matrix<NumRows, NumCols, Real> const& M)
    {
        Matrix<NumRows, NumCols, Real> result;
        for (int32_t r = 0; r < NumRows; ++r)
        {
            for (int32_t c = 0; c < NumCols; ++c)
            {
                result(r, c) = D[r] * M(r, c);
            }
        }
        return result;
    }

    // U*V^T, U is NumRows-by-1, V is Num-Cols-by-1, result is NumRows-by-NumCols.
    template <int32_t NumRows, int32_t NumCols, typename Real>
    Matrix<NumRows, NumCols, Real> OuterProduct(
        Vector<NumRows, Real> const& U, Vector<NumCols, Real> const& V)
    {
        Matrix<NumRows, NumCols, Real> result;
        for (int32_t r = 0; r < NumRows; ++r)
        {
            for (int32_t c = 0; c < NumCols; ++c)
            {
                result(r, c) = U[r] * V[c];
            }
        }
        return result;
    }

    // Initialization to a diagonal matrix whose diagonal entries are the
    // components of D.
    template <int32_t N, typename Real>
    void MakeDiagonal(Vector<N, Real> const& D, Matrix<N, N, Real>& M)
    {
        for (int32_t i = 0; i < N * N; ++i)
        {
            M[i] = (Real)0;
        }

        for (int32_t i = 0; i < N; ++i)
        {
            M(i, i) = D[i];
        }
    }

    // Create an (N+1)-by-(N+1) matrix H by setting the upper N-by-N block to
    // the input N-by-N matrix and all other entries to 0 except for the last
    // row and last column entry which is set to 1.
    template <int32_t N, typename Real>
    Matrix<N + 1, N + 1, Real> HLift(Matrix<N, N, Real> const& M)
    {
        Matrix<N + 1, N + 1, Real> result;
        result.MakeIdentity();
        for (int32_t r = 0; r < N; ++r)
        {
            for (int32_t c = 0; c < N; ++c)
            {
                result(r, c) = M(r, c);
            }
        }
        return result;
    }

    // Extract the upper (N-1)-by-(N-1) block of the input N-by-N matrix.
    template <int32_t N, typename Real>
    Matrix<N - 1, N - 1, Real> HProject(Matrix<N, N, Real> const& M)
    {
        static_assert(N >= 2, "Invalid matrix dimension.");
        Matrix<N - 1, N - 1, Real> result;
        for (int32_t r = 0; r < N - 1; ++r)
        {
            for (int32_t c = 0; c < N - 1; ++c)
            {
                result(r, c) = M(r, c);
            }
        }
        return result;
    }
}
