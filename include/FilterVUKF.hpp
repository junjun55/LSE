/*!
* @file 	FilterVUKF.hpp
* @author 	Michael Blösch
* @date		10.10.2012
* @brief	VUKF Filter for legged robots
 */

#ifndef FILTERVUKF_HPP_
#define FILTERVUKF_HPP_

#include "FilterBase.hpp"
#include "Common.hpp"
#include <Eigen/Dense>
#include "Rotations.hpp"

namespace LSE {

class Manager;

/*! Observability Constrained Extended Kalman Filter */
class FilterVUKF: public FilterBase{
public:public:
	/* -------------------- Constructor/Destructor --------------------- */
	/*! Constructor
	 * @param[in]	pManager	pointer to main class Manager
	 * @param[in]	pFilename	filename of parameter-file
	 */
	FilterVUKF(Manager* pManager,const char* pFilename);
	/*! Destructor */
	virtual ~FilterVUKF();

	/* -------------------- Filter handling --------------------- */
	/*! Updates the filter to time t
	 * @param[in]	t	desired update time
	 */
	virtual void update(const double& t);
	/*! Updates the filter to the newest measurement time */
	virtual void update();
	/*! Return current estimate of robot state (main body)
	 * @return	current robot state
	 */
	virtual State getEst();
	/*! Resets the filter
	 * @param[in]	t	time used to initialize new state estimate
	 */
	virtual void resetEstimate(const double& t);

private:
	typedef Eigen::Matrix<double,30,30> Matrix30d;
	/*! Loads overall parameters from parameter file
	 * @param[in]	pFilename	name of parameter file
	 */
	void loadParam(const char* pFilename);
	/*! Structure of filter intern state */
	struct AugmentedState{
		EIGEN_MAKE_ALIGNED_OPERATOR_NEW

		/*! Position estimate */
		Eigen::Vector3d r_;
		/*! Velocity estimate */
		Eigen::Vector3d v_;
		/*! Attitude estimate (quaternion) */
		Rotations::Quat q_;
		/*! Estimate of accelerometer bias */
		Eigen::Vector3d bf_;
		/*! Estimate of gyroscope bias */
		Eigen::Vector3d bw_;
		/*! Noise on position prediction */
		Eigen::Vector3d nr_;
		/*! Current corrected accelerometer measurement */
		Eigen::Vector3d f_;
		/*! Rotational rate estimate (bias corrected) */
		Eigen::Vector3d w_;
		/*! Random walk of accelerometer bias */
		Eigen::Vector3d nbf_;
		/*! Random walk of gyroscope bias */
		Eigen::Vector3d nbw_;
	};
	/*! Structure of filter intern state */
	struct InternState{
		EIGEN_MAKE_ALIGNED_OPERATOR_NEW

		/*! Time of estimate */
		double t_;
		/*! State estimate */
		AugmentedState x_;
		/*! Sigma points of state */
		AugmentedState X_[1+4*(LSE_VUKF_N)];
		/*! Flag if Sigma points samples */
		bool mbSigmaSampled_;
		/*! Contact flag counter */
		CF CFC_;
		/*! Legs used for kinematic update */
		CF LegArray_;
		/*! Estimate of covariance matrix */
		Eigen::Matrix<double,15,15> P_;

		/* -------------------- Operator overloading --------------------- */
		/*! Assignement operator overloading */
		InternState& operator= (const InternState& x) {
			t_ = x.t_;
			x_ = x.x_;
			for(int i=0;i<1+4*(LSE_VUKF_N);i++){
				X_[i] = x.X_[i];
			}
			CFC_ = x.CFC_;
			LegArray_ = x.LegArray_;
			P_ = x.P_;
			return *this;
		}
	};

	/* -------------------- Filtering/Predicting/Updating --------------------- */
	/*! Filters the referenced internal state up to the given time
	 * @param[in/out]	x	Filter state to be filtered
	 * @param[in]		t	Desired filter time
	 */
	void filterState(InternState& x,const double& t);
	/*! Predicts the referenced internal state using the given IMU measurement
	 * @param[in/out]	x	Filter state to be filtered
	 * @param[in]		t	Desired filter time
	 * @param[in]		m	IMU measurement
	 */
	void predictState(InternState& x,const double& t, const ImuMeas& m);
	/*! Updates the referenced internal state using the given Encoder measurement
	 * @param[in/out]	x	Filter state to be filtered
	 * @param[in]		m	Encoder measurement
	 */
	void encUpdateState(InternState& x,const EncMeas& m);

	/* -------------------- Pointers and filter states --------------------- */
	/*! Pointer to main class Manager */
	Manager* pManager_;
	/*! Safe state (where the chance is high that all measurements have arrived) */
	InternState xs_;
	/*! Predicted state */
	InternState xp_;

	/* -------------------- Parameters (can be specified in the parameter file) --------------------- */
	/*! Initialization state */
	InternState xInit_;
	/*! Predicition noise of position */
	Eigen::Matrix3d Wr_;
	/*! Predicition noise of accelerometer bias */
	Eigen::Matrix3d Wbf_;
	/*! Predicition noise of gyroscope bias */
	Eigen::Matrix3d Wbw_;

	/* -------------------- Parameters of unscented filter --------------------- */
	/*! Alpha */
	double UKFAlpha_;
	/*! Kappa */
	double UKFKappa_;
	/*! Beta */
	double UKFBeta_;
	/*! Weights */
	double UKFWs_,UKFWc_,UKFWi_;
	/*! Lambda */
	double UKFLambda_,UKFGamma_;

	/* -------------------- Flag for modes (can be specified in the parameter file) --------------------- */
	/*! True if accelerometer bias is co-estimated */
	bool mbEstimateAccBias_;
	/*! True if gyroscope bias is co-estimated */
	bool mbEstimateRotBias_;
	/*! True if kinematic measurements are used */
	bool mbUseKin_;

	int counter_;
};

}

#endif /* FILTERVUKF_HPP_ */