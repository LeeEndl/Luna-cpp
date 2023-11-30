/*
 * color palette. made by LeeEndl
 * goal is to make color blending legible by english readers.
 * 
 * though it is incomplete my goal is to merge primary colors (RGB scheme) using math operators. e.g. red() + blue() -> outcome: purple.
 * and yes. this will also work with subtraction. (red() + blue()) - blue() -> outcome: red.
 * ultimatly this should in thoery work for more complex equations. e.g. ({0}, {0}, {0}) + blue() - (255 / 6) -> outcome: black-blue (dark blue)
 *                                                                        |
 *                                                                         ---> nullfying all colors is black. TODO: make more legible like black()
*/
#include <vector>
#include <algorithm>

class red {
protected:
	double R;
public:
	operator double& () noexcept {
		return R;
	};
	red() : R(255) {};
	red(double val) noexcept : R(std::clamp<double>(std::move(val), 0.0, 255.0)) {};
	red operator/(double val) {
		return R / std::move(val);
	}
};
class green {
protected:
	double G;
public:
	operator double& () {
		return G;
	};
	green() : G(255) {};
	green(double val) noexcept : G(std::clamp<double>(std::move(val), 0.0, 255.0)) {};
	green operator/(double val) {
		return G / std::move(val);
	}
};
class blue {
protected:
	double B;
public:
	operator double& () {
		return B;
	};
	blue() : B(255) {};
	blue(double val) noexcept : B(std::clamp<double>(std::move(val), 0.0, 255.0)) {};
	blue operator/(double val) {
		return B / std::move(val);
	}
};
class palette {
protected:
	std::vector<double> P;
public:
	operator std::vector<double>& () {
		return P;
	};
	palette(blue B, green G = { 0 }, red R = { 0 }, double alpha = 255) {
		this->P = { std::move(R), std::move(G), std::move(B), alpha };
	}
	palette(std::vector<double> val) {
		val[3] = 255;
		this->P = std::move(val);
	}
	double operator[](short pos) {
		return this->P[pos];
	}
};