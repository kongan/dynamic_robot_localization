#pragma once

/**\file localization.hpp
 * \brief Description...
 *
 * @version 1.0
 * @author Carlos Miguel Correia da Costa
 */

// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>   <includes>   <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<
#include <dynamic_robot_localization/localization/localization.h>
// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>   </includes>  <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<



namespace dynamic_robot_localization {
// =============================================================================  <public-section>  ============================================================================
// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>   <constructors-destructor>   <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<
template<typename PointT>
Localization<PointT>::Localization() :
	reference_pointcloud_normalize_normals_(true),
	flip_normals_using_occupancy_grid_analysis_(true),
	map_update_mode_(NoIntegration),
	use_incremental_map_update_(false),
	override_pointcloud_timestamp_to_current_time_(false),
	minimum_number_of_points_in_ambient_pointcloud_(10),
	minimum_number_of_points_in_reference_pointcloud_(10),
	localization_detailed_use_millimeters_in_root_mean_square_error_inliers_(false),
	localization_detailed_use_millimeters_in_root_mean_square_error_of_last_registration_correspondences_(false),
	localization_detailed_use_millimeters_in_translation_corrections_(false),
	localization_detailed_use_degrees_in_rotation_corrections_(false),
	localization_detailed_compute_pose_corrections_from_initial_and_final_pose_tfs_(true),
	save_reference_pointclouds_in_binary_format_(true),
	republish_reference_pointcloud_after_successful_registration_(false),
	publish_tf_map_odom_(false),
	publish_tf_when_resetting_initial_pose_(false),
	add_odometry_displacement_(false),
	use_filtered_cloud_as_normal_estimation_surface_ambient_(false),
	use_filtered_cloud_as_normal_estimation_surface_reference_(false),
	compute_normals_when_tracking_pose_(false),
	compute_normals_when_recovering_pose_tracking_(false),
	compute_normals_when_estimating_initial_pose_(true),
	compute_keypoints_when_tracking_pose_(false),
	compute_keypoints_when_recovering_pose_tracking_(false),
	compute_keypoints_when_estimating_initial_pose_(true),
	compute_inliers_angular_distribution_(true),
	compute_outliers_angular_distribution_(true),
	inliers_angular_distribution_(-1.0),
	outliers_angular_distribution_(-1.0),
	last_pose_weighted_mean_filter_(-1.0),
	last_scan_time_(0),
	last_map_received_time_(0),
	last_accepted_pose_time_(ros::Time::now()),
	robot_initial_pose_available_(true),
	pose_tracking_minimum_number_of_failed_registrations_since_last_valid_pose_(25),
	pose_tracking_maximum_number_of_failed_registrations_since_last_valid_pose_(50),
	pose_tracking_recovery_minimum_number_of_failed_registrations_since_last_valid_pose_(3),
	pose_tracking_recovery_maximum_number_of_failed_registrations_since_last_valid_pose_(5),
	pose_tracking_number_of_failed_registrations_since_last_valid_pose_(0),
	reference_pointcloud_loaded_(false),
	reference_pointcloud_2d_(false),
	reference_pointcloud_available_(true),
	reference_pointcloud_required_(true),
	ignore_height_corrections_(false),
	last_accepted_pose_valid_(false),
	last_accepted_pose_performed_tracking_reset_(false),
	received_external_initial_pose_estimation_(false),
	use_internal_tracking_(true),
	last_accepted_pose_base_link_to_map_(tf2::Transform::getIdentity()),
	last_accepted_pose_odom_to_map_(tf2::Transform::getIdentity()),
	sensor_data_processing_status_(WaitingForSensorData),
	pose_to_tf_publisher_(new pose_to_tf_publisher::PoseToTFPublisher(ros::Duration(600))),
	ambient_pointcloud_subscribers_active_(false),
	limit_of_pointclouds_to_process_(-1),
	number_of_processed_pointclouds_(0),
	reference_pointcloud_(new pcl::PointCloud<PointT>()),
	reference_pointcloud_keypoints_(new pcl::PointCloud<PointT>()),
	last_number_points_inserted_in_circular_buffer_(0),
	reference_pointcloud_search_method_(new pcl::search::KdTree<PointT>()),
	number_of_registration_iterations_for_all_matchers_(0),
	correspondence_estimation_time_for_all_matchers_(0),
	transformation_estimation_time_for_all_matchers_(0),
	transform_cloud_time_for_all_matchers_(0),
	cloud_align_time_for_all_matchers_(0),
	root_mean_square_error_of_last_registration_correspondences_(0.0),
	number_correspondences_last_registration_algorithm_(-1),
	outlier_percentage_(0.0),
	number_inliers_(0),
	root_mean_square_error_inliers_(0.0),
	outlier_percentage_reference_pointcloud_(0.0),
	number_inliers_reference_pointcloud_(0),
	root_mean_square_error_inliers_reference_pointcloud_(0.0),
	publish_filtered_pointcloud_only_if_there_is_subscribers_(true),
	publish_aligned_pointcloud_only_if_there_is_subscribers_(true) {}

template<typename PointT>
Localization<PointT>::~Localization() {}
// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>   </constructors-destructor>  <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<



// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>   <Localization-functions>   <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<
template<typename PointT>
void Localization<PointT>::setupConfigurationFromParameterServer(ros::NodeHandlePtr& node_handle, ros::NodeHandlePtr& private_node_handle, std::string configuration_namespace) {
	node_handle_ = node_handle;
	private_node_handle_ = private_node_handle;
	configuration_namespace_ = configuration_namespace;

	// general configurations
	setupMessageManagement(configuration_namespace);
	setupGeneralConfigurations(configuration_namespace);
	setupSubscribeTopicNames(configuration_namespace);
	setupServiceServersNames(configuration_namespace);
	setupPublishTopicNames(configuration_namespace);
	setupFrameIds(configuration_namespace);

	// localization pipeline configurations
	setupReferencePointCloud(configuration_namespace);
	setupFiltersConfigurations(configuration_namespace);
	setupNormalEstimatorsConfigurations(configuration_namespace);
	setupCurvatureEstimatorsConfigurations(configuration_namespace);
	setupKeypointDetectors(configuration_namespace);
	setupCloudMatchersConfigurations(configuration_namespace);
	setupInitialPoseEstimatorsFeatureMatchers(configuration_namespace);
	setupInitialPoseEstimatorsPointMatchers(configuration_namespace);
	setupTrackingMatchers(configuration_namespace);
	setupTrackingRecoveryMatchers(configuration_namespace);
	setupTransformationAlignerConfigurations(configuration_namespace);
	setupOutlierDetectorsConfigurations(configuration_namespace);
	setupOutlierDetectorsConfigurationsReferencePointCloud(configuration_namespace);
	setupCloudAnalyzersConfigurations(configuration_namespace);
	setupTransformationValidatorsForInitialAlignment(configuration_namespace);
	setupTransformationValidatorsForTracking(configuration_namespace);
	setupTransformationValidatorsForTrackingRecovery(configuration_namespace);
	setupRegistrationCovarianceEstimatorsConfigurations(configuration_namespace);
	setupTFPublisher(configuration_namespace);
	updateNormalsEstimationFlags();
	loadReferencePointCloud();
}


template<typename PointT>
bool Localization<PointT>::reloadConfigurationFromParameterServerServiceCallback(dynamic_robot_localization::ReloadLocalizationConfiguration::Request& request, dynamic_robot_localization::ReloadLocalizationConfiguration::Response& response) {
	bool status = reloadConfigurationFromParameterServer(request.localization_configuration);
	response.status = status;
	return status;
}


template<typename PointT>
bool Localization<PointT>::reloadConfigurationFromParameterServer(const dynamic_robot_localization::LocalizationConfiguration& localization_configuration) {
	std::string parsed_string;

	if (parseConfigurationNamespace(localization_configuration.message_management, parsed_string))
		setupMessageManagement(parsed_string);

	if (parseConfigurationNamespace(localization_configuration.general_configurations, parsed_string))
		setupGeneralConfigurations(parsed_string);

	bool setup_subscribe_topic_names = parseConfigurationNamespace(localization_configuration.subscribe_topic_names, parsed_string);
	if (setup_subscribe_topic_names)
		setupSubscribeTopicNames(parsed_string);

	bool setup_service_servers_names = parseConfigurationNamespace(localization_configuration.service_servers_names, parsed_string);
	if (setup_service_servers_names)
		setupServiceServersNames(parsed_string);

	bool setup_publish_topic_names = parseConfigurationNamespace(localization_configuration.publish_topic_names, parsed_string);
	if (setup_publish_topic_names)
		setupPublishTopicNames(parsed_string);

	if (parseConfigurationNamespace(localization_configuration.frame_ids, parsed_string))
		setupFrameIds(parsed_string);

	bool setup_reference_cloud = parseConfigurationNamespace(localization_configuration.reference_pointcloud, parsed_string);
	if (setup_reference_cloud)
		setupReferencePointCloud(parsed_string);

	if (parseConfigurationNamespace(localization_configuration.filters, parsed_string))
		setupFiltersConfigurations(parsed_string);

	if (parseConfigurationNamespace(localization_configuration.normal_estimators, parsed_string))
		setupNormalEstimatorsConfigurations(parsed_string);

	if (parseConfigurationNamespace(localization_configuration.curvature_estimators, parsed_string))
		setupCurvatureEstimatorsConfigurations(parsed_string);

	if (parseConfigurationNamespace(localization_configuration.keypoint_detectors, parsed_string))
		setupKeypointDetectors(parsed_string);

	if (parseConfigurationNamespace(localization_configuration.cloud_matchers_configurations, parsed_string))
		setupCloudMatchersConfigurations(parsed_string);

	if (parseConfigurationNamespace(localization_configuration.initial_pose_estimators_feature_matchers, parsed_string))
		setupInitialPoseEstimatorsFeatureMatchers(parsed_string);

	if (parseConfigurationNamespace(localization_configuration.initial_pose_estimators_point_matchers, parsed_string))
		setupInitialPoseEstimatorsPointMatchers(parsed_string);

	if (parseConfigurationNamespace(localization_configuration.tracking_matchers, parsed_string))
		setupTrackingMatchers(parsed_string);

	if (parseConfigurationNamespace(localization_configuration.tracking_recovery_matchers, parsed_string))
		setupTrackingRecoveryMatchers(parsed_string);

	if (parseConfigurationNamespace(localization_configuration.transformation_aligner, parsed_string))
		setupTransformationAlignerConfigurations(parsed_string);

	if (parseConfigurationNamespace(localization_configuration.outlier_detectors, parsed_string))
		setupOutlierDetectorsConfigurations(parsed_string);

	if (parseConfigurationNamespace(localization_configuration.outlier_detectors_reference_pointcloud, parsed_string))
		setupOutlierDetectorsConfigurationsReferencePointCloud(parsed_string);

	if (parseConfigurationNamespace(localization_configuration.cloud_analyzers, parsed_string))
		setupCloudAnalyzersConfigurations(parsed_string);

	if (parseConfigurationNamespace(localization_configuration.transformation_validators_for_initial_alignment, parsed_string))
		setupTransformationValidatorsForInitialAlignment(parsed_string);

	if (parseConfigurationNamespace(localization_configuration.transformation_validators_for_tracking, parsed_string))
		setupTransformationValidatorsForTracking(parsed_string);

	if (parseConfigurationNamespace(localization_configuration.transformation_validators_for_tracking_recovery, parsed_string))
		setupTransformationValidatorsForTrackingRecovery(parsed_string);

	if (parseConfigurationNamespace(localization_configuration.registration_covariance_estimators, parsed_string))
		setupRegistrationCovarianceEstimatorsConfigurations(parsed_string);

	if (parseConfigurationNamespace(localization_configuration.tf_publisher, parsed_string))
		setupTFPublisher(parsed_string);

	updateNormalsEstimationFlags();

	bool status = true;
	if (setup_reference_cloud)
		status = loadReferencePointCloud();

	if (setup_publish_topic_names) {
		startPublishers();
		startReferenceCloudSubscribers();
	}

	if (setup_subscribe_topic_names)
		startSubscribers();

	if (setup_service_servers_names)
		startServiceServers();

	return status;
}


template<typename PointT>
bool Localization<PointT>::parseConfigurationNamespace(const std::string& configuration_namespace, std::string& configuration_namespace_parsed_out) {
	configuration_namespace_parsed_out.clear();

	if (configuration_namespace.empty())
		return false;

	if (configuration_namespace == "~") {
		return true;
	}

	configuration_namespace_parsed_out = configuration_namespace;
	if (configuration_namespace.back() != '/') {
		configuration_namespace_parsed_out += '/';
	}

	return true;
}

template<typename PointT>
void Localization<PointT>::setupGeneralConfigurations(const std::string& configuration_namespace) {
	ROS_DEBUG_STREAM("Loading [general_configurations] configurations from parameter server namespace [" << (configuration_namespace.empty() ? "~" : configuration_namespace) << "]");
	private_node_handle_->param(configuration_namespace + "general_configurations/publish_tf_map_odom", publish_tf_map_odom_, false);
	private_node_handle_->param(configuration_namespace + "general_configurations/publish_tf_when_resetting_initial_pose", publish_tf_when_resetting_initial_pose_, false);
	private_node_handle_->param(configuration_namespace + "general_configurations/add_odometry_displacement", add_odometry_displacement_, false);
}


template<typename PointT>
void Localization<PointT>::setupSubscribeTopicNames(const std::string &configuration_namespace) {
	ROS_DEBUG_STREAM("Loading [subscribe_topic_names] configurations from parameter server namespace [" << (configuration_namespace.empty() ? "~" : configuration_namespace) << "]");
	private_node_handle_->param(configuration_namespace + "subscribe_topic_names/pose_topic", pose_topic_, std::string("initial_pose"));
	private_node_handle_->param(configuration_namespace + "subscribe_topic_names/pose_stamped_topic", pose_stamped_topic_, std::string("initial_pose_stamped"));
	private_node_handle_->param(configuration_namespace + "subscribe_topic_names/pose_with_covariance_stamped_topic", pose_with_covariance_stamped_topic_, std::string("/initialpose"));
	private_node_handle_->param(configuration_namespace + "subscribe_topic_names/ambient_pointcloud_topic", ambient_pointcloud_topics_, std::string("ambient_pointcloud"));
	private_node_handle_->param(configuration_namespace + "subscribe_topic_names/reference_costmap_topic", reference_costmap_topic_, std::string("/map"));
	private_node_handle_->param(configuration_namespace + "subscribe_topic_names/reference_pointcloud_topic", reference_pointcloud_topic_, std::string(""));
}

template<typename PointT>
void Localization<PointT>::setupServiceServersNames(const std::string &configuration_namespace) {
	ROS_DEBUG_STREAM("Loading [service_servers_names] configurations from parameter server namespace [" << (configuration_namespace.empty() ? "~" : configuration_namespace) << "]");
	private_node_handle_->param(configuration_namespace + "service_servers_names/reload_localization_configuration_service_server_name", reload_localization_configuration_service_server_name_, std::string("reload_localization_configuration"));
}

template<typename PointT>
void Localization<PointT>::setupPublishTopicNames(const std::string& configuration_namespace) {
	ROS_DEBUG_STREAM("Loading [publish_topic_names] configurations from parameter server namespace [" << (configuration_namespace.empty() ? "~" : configuration_namespace) << "]");
	private_node_handle_->param(configuration_namespace + "publish_topic_names/publish_filtered_pointcloud_only_if_there_is_subscribers", publish_filtered_pointcloud_only_if_there_is_subscribers_, true);
	private_node_handle_->param(configuration_namespace + "publish_topic_names/publish_aligned_pointcloud_only_if_there_is_subscribers", publish_aligned_pointcloud_only_if_there_is_subscribers_, true);
	private_node_handle_->param(configuration_namespace + "publish_topic_names/reference_pointcloud_publish_topic", reference_pointcloud_publish_topic_, std::string("reference_pointcloud"));
	private_node_handle_->param(configuration_namespace + "publish_topic_names/reference_pointcloud_keypoints_publish_topic", reference_pointcloud_keypoints_publish_topic_, std::string("reference_pointcloud_keypoints"));
	private_node_handle_->param(configuration_namespace + "publish_topic_names/filtered_pointcloud_publish_topic", filtered_pointcloud_publish_topic_, std::string("filtered_pointcloud"));
	private_node_handle_->param(configuration_namespace + "publish_topic_names/aligned_pointcloud_publish_topic", aligned_pointcloud_publish_topic_, std::string("aligned_pointcloud"));
	private_node_handle_->param(configuration_namespace + "publish_topic_names/pose_with_covariance_stamped_publish_topic", pose_with_covariance_stamped_publish_topic_, std::string("localization_pose_with_covariance"));
	private_node_handle_->param(configuration_namespace + "publish_topic_names/pose_with_covariance_stamped_tracking_reset_publish_topic", pose_with_covariance_stamped_tracking_reset_publish_topic_, std::string("initial_pose_with_covariance"));
	private_node_handle_->param(configuration_namespace + "publish_topic_names/pose_stamped_publish_topic", pose_stamped_publish_topic_, std::string("localization_pose"));
	private_node_handle_->param(configuration_namespace + "publish_topic_names/pose_array_publish_topic", pose_array_publish_topic_, std::string("localization_initial_pose_estimations"));
	private_node_handle_->param(configuration_namespace + "publish_topic_names/localization_detailed_publish_topic", localization_detailed_publish_topic_, std::string("localization_detailed"));
	private_node_handle_->param(configuration_namespace + "publish_topic_names/localization_diagnostics_publish_topic", localization_diagnostics_publish_topic_, std::string("diagnostics"));
	private_node_handle_->param(configuration_namespace + "publish_topic_names/localization_times_publish_topic", localization_times_publish_topic_, std::string("localization_times"));
}


template<typename PointT>
void Localization<PointT>::setupFrameIds(const std::string& configuration_namespace) {
	ROS_DEBUG_STREAM("Loading [frame_ids] configurations from parameter server namespace [" << (configuration_namespace.empty() ? "~" : configuration_namespace) << "]");
	private_node_handle_->param(configuration_namespace + "frame_ids/map_frame_id", map_frame_id_, std::string("map"));
	private_node_handle_->param(configuration_namespace + "frame_ids/map_frame_id_for_transforming_pointclouds", map_frame_id_for_transforming_pointclouds_, map_frame_id_);
	private_node_handle_->param(configuration_namespace + "frame_ids/map_frame_id_for_publishing_pointclouds", map_frame_id_for_publishing_pointclouds_, map_frame_id_);
	private_node_handle_->param(configuration_namespace + "frame_ids/odom_frame_id", odom_frame_id_, std::string("odom"));
	private_node_handle_->param(configuration_namespace + "frame_ids/base_link_frame_id", base_link_frame_id_, std::string("base_footprint"));
	private_node_handle_->param(configuration_namespace + "frame_ids/sensor_frame_id", sensor_frame_id_, std::string("hokuyo_front_laser_link"));
}


template<typename PointT>
void Localization<PointT>::setupInitialPose() {
	setupInitialPose(configuration_namespace_, ros::Time::now(), true);
}


template<typename PointT>
void Localization<PointT>::setupInitialPose(const std::string& configuration_namespace, const ros::Time& time, bool use_latest_tf_time) {
	double x, y, z, roll, pitch ,yaw, qx, qy, qz, qw;
	private_node_handle_->param(configuration_namespace + "initial_pose/position/x", x, 0.0);
	private_node_handle_->param(configuration_namespace + "initial_pose/position/y", y, 0.0);
	private_node_handle_->param(configuration_namespace + "initial_pose/position/z", z, 0.0);
	private_node_handle_->param(configuration_namespace + "initial_pose/orientation_rpy/roll", roll, 0.0);
	private_node_handle_->param(configuration_namespace + "initial_pose/orientation_rpy/pitch", pitch, 0.0);
	private_node_handle_->param(configuration_namespace + "initial_pose/orientation_rpy/yaw", yaw, 0.0);
	private_node_handle_->param(configuration_namespace + "initial_pose/orientation_quaternion/x", qx, -1.0);
	private_node_handle_->param(configuration_namespace + "initial_pose/orientation_quaternion/y", qy, -1.0);
	private_node_handle_->param(configuration_namespace + "initial_pose/orientation_quaternion/z", qz, -1.0);
	private_node_handle_->param(configuration_namespace + "initial_pose/orientation_quaternion/w", qw, -1.0);

	last_accepted_pose_base_link_to_map_.setOrigin(tf2::Vector3(x, y, z));

	tf2::Quaternion orientation;
	if ((qx + qy +qz + qw) < 0) {
		orientation.setRPY(roll, pitch, yaw);
	} else {
		orientation.setValue(qx, qy, qz, qw);
	}
	orientation.normalize();
	last_accepted_pose_base_link_to_map_.setRotation(orientation);

	if (!math_utils::isTransformValid(last_accepted_pose_base_link_to_map_)) {
		ROS_WARN("Discarded initial pose with NaN values (set to identity)!");
		last_accepted_pose_base_link_to_map_ = tf2::Transform::getIdentity();
	}

	ros::Time::waitForValid();

	bool robot_initial_pose_in_base_to_map;
	private_node_handle_->param(configuration_namespace + "initial_pose/robot_initial_pose_in_base_to_map", robot_initial_pose_in_base_to_map, false);

	private_node_handle_->param(configuration_namespace + "initial_pose/robot_initial_pose_available", robot_initial_pose_available_, true);

	private_node_handle_->param(configuration_namespace + "initial_pose/reset_initial_pose_when_tracking_is_lost", reset_initial_pose_when_tracking_is_lost_, false);

	tf2::Transform transform_odom_to_base_link;
	ROS_DEBUG_STREAM("Looking for TF [ " << base_link_frame_id_ << " -> " << odom_frame_id_ << " ]");
	bool tf_lookup_succeeded = false;
	if (use_latest_tf_time) {
		tf_lookup_succeeded = pose_to_tf_publisher_->getTfCollector().lookForLatestTransform(transform_odom_to_base_link, base_link_frame_id_, odom_frame_id_, ros::Duration(10));
	} else {
		tf_lookup_succeeded = pose_to_tf_publisher_->getTfCollector().lookForTransform(transform_odom_to_base_link, base_link_frame_id_, odom_frame_id_, time, ros::Duration(10));
	}

	if (!tf_lookup_succeeded || !math_utils::isTransformValid(transform_odom_to_base_link)) {
		transform_odom_to_base_link = tf2::Transform::getIdentity();
		ROS_WARN_STREAM("Failed to get tf from " << base_link_frame_id_ << " to " << odom_frame_id_ << " when setting localization initial pose");
	}
	ROS_DEBUG_STREAM("Finished looking for TF [ " << base_link_frame_id_ << " -> " << odom_frame_id_ << " ]");

	if (robot_initial_pose_in_base_to_map) {
		last_accepted_pose_odom_to_map_ = last_accepted_pose_base_link_to_map_ * transform_odom_to_base_link;
	} else {
		last_accepted_pose_odom_to_map_ = last_accepted_pose_base_link_to_map_;
		last_accepted_pose_base_link_to_map_ *= transform_odom_to_base_link.inverse();
	}


	if (robot_initial_pose_available_ && !pose_with_covariance_stamped_tracking_reset_publisher_.getTopic().empty()) {
		geometry_msgs::PoseWithCovarianceStampedPtr pose_msg(new geometry_msgs::PoseWithCovarianceStamped());
		pose_msg->header.frame_id = map_frame_id_;
		pose_msg->header.stamp = time;

		laserscan_to_pointcloud::tf_rosmsg_eigen_conversions::transformTF2ToMsg(last_accepted_pose_base_link_to_map_, pose_msg->pose.pose);
		pose_with_covariance_stamped_tracking_reset_publisher_.publish(pose_msg);
	}

	if (publish_tf_when_resetting_initial_pose_) {
		pose_to_tf_publisher_->publishInitialPoseFromParameterServer();
	}

	last_accepted_pose_time_ = time;
}


template<typename PointT>
void Localization<PointT>::setupTFPublisher(const std::string& configuration_namespace) {
	ROS_DEBUG_STREAM("Loading [tf_publisher] configurations from parameter server namespace [" << (configuration_namespace.empty() ? "~" : configuration_namespace) << "]");
	pose_to_tf_publisher_->setupConfigurationFromParameterServer(node_handle_, private_node_handle_, configuration_namespace + "pose_to_tf_publisher/");
	pose_to_tf_publisher_->setBaseLinkFrameId(base_link_frame_id_);
	pose_to_tf_publisher_->setOdomFrameId(odom_frame_id_);
	pose_to_tf_publisher_->setMapFrameId(map_frame_id_);
}


template<typename PointT>
void Localization<PointT>::setupMessageManagement(const std::string& configuration_namespace) {
	ROS_DEBUG_STREAM("Loading [message_management] configurations from parameter server namespace [" << (configuration_namespace.empty() ? "~" : configuration_namespace) << "]");

	double tf_buffer_duration;
	private_node_handle_->param(configuration_namespace + "message_management/tf_buffer_duration", tf_buffer_duration, 600.0);
	pose_to_tf_publisher_.reset(new pose_to_tf_publisher::PoseToTFPublisher(ros::Duration(tf_buffer_duration)));

	double tf_timeout;
	private_node_handle_->param(configuration_namespace + "message_management/tf_timeout", tf_timeout, 0.5);
	tf_timeout_ = ros::Duration(tf_timeout);

	private_node_handle_->param(configuration_namespace + "message_management/override_pointcloud_timestamp_to_current_time", override_pointcloud_timestamp_to_current_time_, false);

	double max_seconds_ambient_pointcloud_age;
	private_node_handle_->param(configuration_namespace + "message_management/max_seconds_ambient_pointcloud_age", max_seconds_ambient_pointcloud_age, 3.0);
	max_seconds_ambient_pointcloud_age_.fromSec(max_seconds_ambient_pointcloud_age);

	double max_seconds_ambient_pointcloud_offset_to_last_estimated_pose;
	private_node_handle_->param(configuration_namespace + "message_management/max_seconds_ambient_pointcloud_offset_to_last_estimated_pose", max_seconds_ambient_pointcloud_offset_to_last_estimated_pose, 0.0);
	max_seconds_ambient_pointcloud_offset_to_last_estimated_pose_.fromSec(max_seconds_ambient_pointcloud_offset_to_last_estimated_pose);

	double min_seconds_between_scan_registration;
	private_node_handle_->param(configuration_namespace + "message_management/min_seconds_between_scan_registration", min_seconds_between_scan_registration, 0.0);
	min_seconds_between_scan_registration_.fromSec(min_seconds_between_scan_registration);

	double min_seconds_between_reference_pointcloud_update;
	private_node_handle_->param(configuration_namespace + "message_management/min_seconds_between_reference_pointcloud_update", min_seconds_between_reference_pointcloud_update, 5.0);
	min_seconds_between_reference_pointcloud_update_.fromSec(min_seconds_between_reference_pointcloud_update);

	private_node_handle_->param(configuration_namespace + "message_management/minimum_number_of_points_in_ambient_pointcloud", minimum_number_of_points_in_ambient_pointcloud_, 10);

	private_node_handle_->param(configuration_namespace + "message_management/circular_buffer_require_reception_of_pointcloud_msgs_from_all_topics_before_doing_registration", circular_buffer_require_reception_of_pointcloud_msgs_from_all_topics_before_doing_registration_, false);
	private_node_handle_->param(configuration_namespace + "message_management/circular_buffer_clear_inserted_points_if_registration_fails", circular_buffer_clear_inserted_points_if_registration_fails_, false);
	private_node_handle_->param(configuration_namespace + "message_management/minimum_number_points_ambient_pointcloud_circular_buffer", minimum_number_points_ambient_pointcloud_circular_buffer_, 0);
	int maximum_number_points_ambient_pointcloud_circular_buffer;
	private_node_handle_->param(configuration_namespace + "message_management/maximum_number_points_ambient_pointcloud_circular_buffer", maximum_number_points_ambient_pointcloud_circular_buffer, 0);
	ambient_pointcloud_with_circular_buffer_.reset();
	if (maximum_number_points_ambient_pointcloud_circular_buffer > 0) {
		ambient_pointcloud_with_circular_buffer_.reset(new CircularBufferPointCloud<PointT>(maximum_number_points_ambient_pointcloud_circular_buffer));
	}
	private_node_handle_->param(configuration_namespace + "message_management/limit_of_pointclouds_to_process", limit_of_pointclouds_to_process_, -1);

	private_node_handle_->param(configuration_namespace + "message_management/localization_detailed_use_millimeters_in_root_mean_square_error_inliers", localization_detailed_use_millimeters_in_root_mean_square_error_inliers_, false);
	private_node_handle_->param(configuration_namespace + "message_management/localization_detailed_use_millimeters_in_root_mean_square_error_of_last_registration_correspondences", localization_detailed_use_millimeters_in_root_mean_square_error_of_last_registration_correspondences_, false);
	private_node_handle_->param(configuration_namespace + "message_management/localization_detailed_use_millimeters_in_translation_corrections", localization_detailed_use_millimeters_in_translation_corrections_, false);
	private_node_handle_->param(configuration_namespace + "message_management/localization_detailed_use_degrees_in_rotation_corrections", localization_detailed_use_degrees_in_rotation_corrections_, false);
	private_node_handle_->param(configuration_namespace + "message_management/localization_detailed_compute_pose_corrections_from_initial_and_final_pose_tfs", localization_detailed_compute_pose_corrections_from_initial_and_final_pose_tfs_, true);
	private_node_handle_->param(configuration_namespace + "message_management/use_odom_when_transforming_cloud_to_map_frame", use_odom_when_transforming_cloud_to_map_frame_, true);
	private_node_handle_->param(configuration_namespace + "message_management/invert_cloud_to_map_transform", invert_cloud_to_map_transform_, false);
	private_node_handle_->param(configuration_namespace + "message_management/invert_registration_transformation", invert_registration_transformation_, false);
	private_node_handle_->param(configuration_namespace + "message_management/invert_initial_poses_from_msgs", invert_initial_poses_from_msgs_, false);
	private_node_handle_->param(configuration_namespace + "message_management/initial_pose_msg_needs_to_be_in_map_frame", initial_pose_msg_needs_to_be_in_map_frame_, true);
	private_node_handle_->param(configuration_namespace + "message_management/use_base_link_frame_when_publishing_registration_pose", use_base_link_frame_when_publishing_registration_pose_, false);
	private_node_handle_->param(configuration_namespace + "message_management/use_base_link_frame_when_publishing_initial_poses_array", use_base_link_frame_when_publishing_initial_poses_array_, false);
	private_node_handle_->param(configuration_namespace + "message_management/apply_cloud_registration_inverse_to_initial_poses_array", apply_cloud_registration_inverse_to_initial_poses_array_, false);
}


template<typename PointT>
void Localization<PointT>::setupReferencePointCloud(const std::string& configuration_namespace) {
	ROS_DEBUG_STREAM("Loading [reference_pointcloud] configurations from parameter server namespace [" << (configuration_namespace.empty() ? "~" : configuration_namespace) << "]");
	private_node_handle_->param(configuration_namespace + "reference_pointclouds_database_folder_path", reference_pointclouds_database_folder_path_, std::string(""));
	private_node_handle_->param(configuration_namespace + "reference_pointclouds/reference_pointcloud_filename", reference_pointcloud_filename_, std::string(""));
	private_node_handle_->param(configuration_namespace + "reference_pointclouds/normalize_normals", reference_pointcloud_normalize_normals_, true);
	private_node_handle_->param(configuration_namespace + "reference_pointclouds/reference_pointcloud_preprocessed_save_filename", reference_pointcloud_preprocessed_save_filename_, std::string(""));
	private_node_handle_->param(configuration_namespace + "reference_pointclouds/save_reference_pointclouds_in_binary_format", save_reference_pointclouds_in_binary_format_, true);
	private_node_handle_->param(configuration_namespace + "reference_pointclouds/republish_reference_pointcloud_after_successful_registration", republish_reference_pointcloud_after_successful_registration_, false);
	private_node_handle_->param(configuration_namespace + "reference_pointclouds/minimum_number_of_points_in_reference_pointcloud", minimum_number_of_points_in_reference_pointcloud_, 10);

	std::string reference_pointcloud_type;
	private_node_handle_->param(configuration_namespace + "reference_pointclouds/reference_pointcloud_type", reference_pointcloud_type, std::string("3D"));
	if (reference_pointcloud_type == "2D") {
		reference_pointcloud_2d_ = true;
	} else if (reference_pointcloud_type == "3D") {
		reference_pointcloud_2d_ = false;
	}

	private_node_handle_->param(configuration_namespace + "reference_pointclouds/reference_pointcloud_available", reference_pointcloud_available_, true);
	private_node_handle_->param(configuration_namespace + "reference_pointclouds/reference_pointcloud_required", reference_pointcloud_required_, true);

	std::string reference_pointcloud_update_mode;
	private_node_handle_->param(configuration_namespace + "reference_pointclouds/reference_pointcloud_update_mode", reference_pointcloud_update_mode, std::string("NoIntegration"));
	if (reference_pointcloud_update_mode == "NoIntegration") {
		map_update_mode_ = NoIntegration;
	} else if (reference_pointcloud_update_mode == "FullIntegration") {
		map_update_mode_ = FullIntegration;
	} else if (reference_pointcloud_update_mode == "InliersIntegration") {
		map_update_mode_ = InliersIntegration;
	} else if (reference_pointcloud_update_mode == "OutliersIntegration") {
		map_update_mode_ = OutliersIntegration;
	}

	private_node_handle_->param(configuration_namespace + "reference_pointclouds/use_incremental_map_update", use_incremental_map_update_, false);
	reference_pointcloud_->header.frame_id = map_frame_id_for_publishing_pointclouds_;
}


template<typename PointT>
void Localization<PointT>::setupFiltersConfigurations(const std::string& configuration_namespace) {
	ROS_DEBUG_STREAM("Loading [filters] configurations from parameter server namespace [" << (configuration_namespace.empty() ? "~" : configuration_namespace) << "]");

	reference_cloud_filters_.clear();
	ambient_pointcloud_integration_filters_.clear();
	ambient_pointcloud_integration_filters_map_frame_.clear();
	ambient_pointcloud_feature_registration_filters_.clear();
	ambient_pointcloud_map_frame_feature_registration_filters_.clear();
	ambient_pointcloud_filters_.clear();
	ambient_pointcloud_filters_custom_frame_.clear();
	ambient_pointcloud_filters_map_frame_.clear();
	ambient_pointcloud_filters_after_normal_estimation_.clear();

	private_node_handle_->param(configuration_namespace + "filters/ambient_pointcloud_integration_filters_preprocessed_pointcloud_save_filename", ambient_pointcloud_integration_filters_preprocessed_pointcloud_save_filename_, std::string(""));
	private_node_handle_->param(configuration_namespace + "filters/ambient_pointcloud_integration_filters_preprocessed_pointcloud_save_original_pointcloud", ambient_pointcloud_integration_filters_preprocessed_pointcloud_save_original_pointcloud_, true);

	loadFiltersFromParameterServer(reference_cloud_filters_, configuration_namespace + "filters/reference_pointcloud/");
	loadFiltersFromParameterServer(ambient_pointcloud_integration_filters_, configuration_namespace + "filters/ambient_pointcloud_integration_filters/");
	loadFiltersFromParameterServer(ambient_pointcloud_integration_filters_map_frame_, configuration_namespace + "filters/ambient_pointcloud_integration_filters_map_frame/");
	loadFiltersFromParameterServer(ambient_pointcloud_feature_registration_filters_, configuration_namespace + "filters/ambient_pointcloud_feature_registration/");
	loadFiltersFromParameterServer(ambient_pointcloud_map_frame_feature_registration_filters_, configuration_namespace + "filters/ambient_pointcloud_map_frame_feature_registration/");
	loadFiltersFromParameterServer(ambient_pointcloud_filters_, configuration_namespace + "filters/ambient_pointcloud/");
	loadFiltersFromParameterServer(ambient_pointcloud_filters_custom_frame_, configuration_namespace + "filters/ambient_pointcloud_custom_frame/");
	private_node_handle_->param(configuration_namespace + "filters/ambient_pointcloud_custom_frame/custom_frame_id", ambient_pointcloud_filters_custom_frame_id_, std::string(""));
	loadFiltersFromParameterServer(ambient_pointcloud_filters_map_frame_, configuration_namespace + "filters/ambient_pointcloud_map_frame/");
	loadFiltersFromParameterServer(ambient_pointcloud_filters_after_normal_estimation_, configuration_namespace + "filters/ambient_pointcloud_filters_after_normal_estimation/");
}


template<typename PointT>
void Localization<PointT>::loadFiltersFromParameterServer(std::vector< typename CloudFilter<PointT>::Ptr >& filters_container, std::string configuration_namespace) {
	XmlRpc::XmlRpcValue filters;
	if (private_node_handle_->getParam(configuration_namespace, filters) && filters.getType() == XmlRpc::XmlRpcValue::TypeStruct) {
		for (XmlRpc::XmlRpcValue::iterator it = filters.begin(); it != filters.end(); ++it) {
			std::string filter_name = it->first;
			typename CloudFilter<PointT>::Ptr cloud_filter;
			if (filter_name.find("approximate_voxel_grid") != std::string::npos) {
				cloud_filter.reset(new ApproximateVoxelGrid<PointT>());
			} else if (filter_name.find("voxel_grid") != std::string::npos) {
				cloud_filter.reset(new VoxelGrid<PointT>());
			} else if (filter_name.find("pass_through") != std::string::npos) {
				cloud_filter.reset(new PassThrough<PointT>());
			} else if (filter_name.find("radius_outlier_removal") != std::string::npos) {
				cloud_filter.reset(new RadiusOutlierRemoval<PointT>());
			} else if (filter_name.find("crop_box") != std::string::npos) {
				cloud_filter.reset(new CropBox<PointT>());
			} else if (filter_name.find("random_sample") != std::string::npos) {
				cloud_filter.reset(new RandomSample<PointT>());
			} else if (filter_name.find("statistical_outlier_removal") != std::string::npos) {
				cloud_filter.reset(new StatisticalOutlierRemoval<PointT>());
			} else if (filter_name.find("covariance_sampling") != std::string::npos) {
				cloud_filter.reset(new CovarianceSampling<PointT>());
			} else if (filter_name.find("scale") != std::string::npos) {
				cloud_filter.reset(new Scale<PointT>());
			} else if (filter_name.find("plane_segmentation") != std::string::npos) {
				cloud_filter.reset(new PlaneSegmentation<PointT>());
			} else if (filter_name.find("euclidean_clustering") != std::string::npos) {
				cloud_filter.reset(new EuclideanClustering<PointT>());
			} else if (filter_name.find("region_growing") != std::string::npos) {
				cloud_filter.reset(new RegionGrowing<PointT>());
			} else if (filter_name.find("hsv_segmentation") != std::string::npos) {
				cloud_filter.reset(new HSVSegmentation<PointT>());
			}

			if (cloud_filter) {
				cloud_filter->setupConfigurationFromParameterServer(node_handle_, private_node_handle_, configuration_namespace + filter_name + "/");
				filters_container.push_back(cloud_filter);
			}
		}
	}
}


template<typename PointT>
void Localization<PointT>::setupNormalEstimatorsConfigurations(const std::string& configuration_namespace) {
	ROS_DEBUG_STREAM("Loading [normal_estimators] configurations from parameter server namespace [" << (configuration_namespace.empty() ? "~" : configuration_namespace) << "]");
	private_node_handle_->param(configuration_namespace + "normal_estimators/ambient_pointcloud/compute_normals_when_tracking_pose", compute_normals_when_tracking_pose_, false);
	private_node_handle_->param(configuration_namespace + "normal_estimators/ambient_pointcloud/compute_normals_when_recovering_pose_tracking", compute_normals_when_recovering_pose_tracking_, false);
	private_node_handle_->param(configuration_namespace + "normal_estimators/ambient_pointcloud/compute_normals_when_estimating_initial_pose", compute_normals_when_estimating_initial_pose_, true);
	private_node_handle_->param(configuration_namespace + "normal_estimators/reference_pointcloud/use_filtered_cloud_as_normal_estimation_surface", use_filtered_cloud_as_normal_estimation_surface_reference_, false);
	private_node_handle_->param(configuration_namespace + "normal_estimators/reference_pointcloud/flip_normals_using_occupancy_grid_analysis", flip_normals_using_occupancy_grid_analysis_, true);
	private_node_handle_->param(configuration_namespace + "normal_estimators/ambient_pointcloud/use_filtered_cloud_as_normal_estimation_surface", use_filtered_cloud_as_normal_estimation_surface_ambient_, false);
	loadNormalEstimatorFromParameterServer(reference_cloud_normal_estimator_, "normal_estimators/reference_pointcloud/");
	loadNormalEstimatorFromParameterServer(ambient_cloud_normal_estimator_, "normal_estimators/ambient_pointcloud/");
}


template<typename PointT>
void Localization<PointT>::updateNormalsEstimationFlags() {
	for (size_t i = 0; i < tracking_matchers_.size(); ++i) {
		if (tracking_matchers_[i]->registrationRequiresNormalsOnAmbientPointCloud()) {
			compute_normals_when_tracking_pose_ = true;
		}
	}

	for (size_t i = 0; i < tracking_recovery_matchers_.size(); ++i) {
		if (tracking_recovery_matchers_[i]->registrationRequiresNormalsOnAmbientPointCloud()) {
			compute_normals_when_recovering_pose_tracking_ = true;
		}
	}

	for (size_t i = 0; i < initial_pose_estimators_point_matchers_.size(); ++i) {
		if (initial_pose_estimators_point_matchers_[i]->registrationRequiresNormalsOnAmbientPointCloud()) {
			compute_normals_when_estimating_initial_pose_ = true;
		}
	}

	for (size_t i = 0; i < initial_pose_estimators_feature_matchers_.size(); ++i) {
		if (initial_pose_estimators_feature_matchers_[i]->registrationRequiresNormalsOnAmbientPointCloud()) {
			compute_normals_when_estimating_initial_pose_ = true;
		}
	}
}


template<typename PointT>
void Localization<PointT>::setupCurvatureEstimatorsConfigurations(const std::string& configuration_namespace) {
	ROS_DEBUG_STREAM("Loading [curvature_estimators] configurations from parameter server namespace [" << (configuration_namespace.empty() ? "~" : configuration_namespace) << "]");
	loadCurvatureEstimatorFromParameterServer(reference_cloud_curvature_estimator_, configuration_namespace + "curvature_estimators/reference_pointcloud/");
	loadCurvatureEstimatorFromParameterServer(ambient_cloud_curvature_estimator_, configuration_namespace + "curvature_estimators/ambient_pointcloud/");
}


template<typename PointT>
void Localization<PointT>::loadNormalEstimatorFromParameterServer(typename NormalEstimator<PointT>::Ptr& normal_estimator, std::string configuration_namespace) {
	normal_estimator.reset();
	XmlRpc::XmlRpcValue normal_estimators;
	if (private_node_handle_->getParam(configuration_namespace, normal_estimators) && normal_estimators.getType() == XmlRpc::XmlRpcValue::TypeStruct) {
		for (XmlRpc::XmlRpcValue::iterator it = normal_estimators.begin(); it != normal_estimators.end(); ++it) {
			std::string estimator_name = it->first;
			if (estimator_name.find("normal_estimator_sac") != std::string::npos) {
				normal_estimator = typename NormalEstimator<PointT>::Ptr(new NormalEstimatorSAC<PointT>());
			} else if (estimator_name.find("normal_estimation_omp") != std::string::npos) {
				normal_estimator = typename NormalEstimator<PointT>::Ptr(new NormalEstimationOMP<PointT>());
			} else if (estimator_name.find("moving_least_squares") != std::string::npos) {
				normal_estimator = typename NormalEstimator<PointT>::Ptr(new MovingLeastSquares<PointT>());
			}

			if (normal_estimator) {
				normal_estimator->setupConfigurationFromParameterServer(node_handle_, private_node_handle_, configuration_namespace + estimator_name + "/");
				return;
			}
		}
	}
}


template<typename PointT>
void Localization<PointT>::loadCurvatureEstimatorFromParameterServer(typename CurvatureEstimator<PointT>::Ptr& curvature_estimator, std::string configuration_namespace) {
	curvature_estimator.reset();
	XmlRpc::XmlRpcValue curvature_estimators;
	if (private_node_handle_->getParam(configuration_namespace, curvature_estimators) && curvature_estimators.getType() == XmlRpc::XmlRpcValue::TypeStruct) {
		for (XmlRpc::XmlRpcValue::iterator it = curvature_estimators.begin(); it != curvature_estimators.end(); ++it) {
			std::string estimator_name = it->first;
			if (estimator_name.find("principal_curvatures_estimation") != std::string::npos) {
				curvature_estimator = typename CurvatureEstimator<PointT>::Ptr(new PrincipalCurvaturesEstimation<PointT>());
			}

			if (curvature_estimator) {
				curvature_estimator->setupConfigurationFromParameterServer(node_handle_, private_node_handle_, configuration_namespace + estimator_name + "/");
				return;
			}
		}
	}
}


template<typename PointT>
void Localization<PointT>::setupKeypointDetectors(const std::string& configuration_namespace) {
	ROS_DEBUG_STREAM("Loading [keypoint_detectors] configurations from parameter server namespace [" << (configuration_namespace.empty() ? "~" : configuration_namespace) << "]");

	reference_cloud_keypoint_detectors_.clear();
	ambient_cloud_keypoint_detectors_.clear();

	private_node_handle_->param(configuration_namespace + "keypoint_detectors/reference_pointcloud/reference_pointcloud_keypoints_filename", reference_pointcloud_keypoints_filename_, std::string(""));
	private_node_handle_->param(configuration_namespace + "keypoint_detectors/reference_pointcloud/reference_pointcloud_keypoints_save_filename", reference_pointcloud_keypoints_save_filename_, std::string(""));
	private_node_handle_->param(configuration_namespace + "keypoint_detectors/ambient_pointcloud/compute_keypoints_when_tracking_pose", compute_keypoints_when_tracking_pose_, false);
	private_node_handle_->param(configuration_namespace + "keypoint_detectors/ambient_pointcloud/compute_keypoints_when_recovering_pose_tracking", compute_keypoints_when_recovering_pose_tracking_, false);
	private_node_handle_->param(configuration_namespace + "keypoint_detectors/ambient_pointcloud/compute_keypoints_when_estimating_initial_pose", compute_keypoints_when_estimating_initial_pose_, true);

	loadKeypointDetectorsFromParameterServer(reference_cloud_keypoint_detectors_, configuration_namespace + "keypoint_detectors/reference_pointcloud/");
	loadKeypointDetectorsFromParameterServer(ambient_cloud_keypoint_detectors_, configuration_namespace + "keypoint_detectors/ambient_pointcloud/");
}


template<typename PointT>
void Localization<PointT>::loadKeypointDetectorsFromParameterServer(std::vector<typename KeypointDetector<PointT>::Ptr >& keypoint_detectors, std::string configuration_namespace) {
	XmlRpc::XmlRpcValue detectors;
	if (private_node_handle_->getParam(configuration_namespace, detectors) && detectors.getType() == XmlRpc::XmlRpcValue::TypeStruct) {
		for (XmlRpc::XmlRpcValue::iterator it = detectors.begin(); it != detectors.end(); ++it) {
			std::string detector_name = it->first;
			typename KeypointDetector<PointT>::Ptr keypoint_detector;
			if (detector_name.find("intrinsic_shape_signature_3d") != std::string::npos) {
				keypoint_detector.reset(new IntrinsicShapeSignature3D<PointT>());
			} else if (detector_name.find("sift_3d") != std::string::npos) {
				keypoint_detector.reset(new SIFT3D<PointT>());
			}

			if (keypoint_detector) {
				keypoint_detector->setupConfigurationFromParameterServer(node_handle_, private_node_handle_, configuration_namespace + detector_name + "/");
				keypoint_detectors.push_back(keypoint_detector);
			}
		}
	}
}


template<typename PointT>
void Localization<PointT>::setupCloudMatchersConfigurations(const std::string& configuration_namespace) {
	ROS_DEBUG_STREAM("Loading [cloud_matchers_configurations] configurations from parameter server namespace [" << (configuration_namespace.empty() ? "~" : configuration_namespace) << "]");

	private_node_handle_->param(configuration_namespace + "tracking_matchers/ignore_height_corrections", ignore_height_corrections_, false);
	private_node_handle_->param(configuration_namespace + "tracking_matchers/use_internal_tracking", use_internal_tracking_, true);
	private_node_handle_->param(configuration_namespace + "tracking_matchers/last_pose_weighted_mean_filter", last_pose_weighted_mean_filter_, -1.0);

	double pose_tracking_timeout;
	private_node_handle_->param(configuration_namespace + "tracking_matchers/pose_tracking_timeout", pose_tracking_timeout, 30.0);
	pose_tracking_timeout_.fromSec(pose_tracking_timeout);

	double pose_tracking_recovery_timeout;
	private_node_handle_->param(configuration_namespace + "tracking_matchers/pose_tracking_recovery_timeout", pose_tracking_recovery_timeout, 0.5);
	pose_tracking_recovery_timeout_.fromSec(pose_tracking_recovery_timeout);

	double initial_pose_estimation_timeout;
	private_node_handle_->param(configuration_namespace + "initial_pose_estimators_matchers/initial_pose_estimation_timeout", initial_pose_estimation_timeout, 600.0);
	initial_pose_estimation_timeout_.fromSec(initial_pose_estimation_timeout);

	private_node_handle_->param(configuration_namespace + "tracking_matchers/pose_tracking_minimum_number_of_failed_registrations_since_last_valid_pose", pose_tracking_minimum_number_of_failed_registrations_since_last_valid_pose_, 25);
	private_node_handle_->param(configuration_namespace + "tracking_matchers/pose_tracking_maximum_number_of_failed_registrations_since_last_valid_pose", pose_tracking_maximum_number_of_failed_registrations_since_last_valid_pose_, 50);
	private_node_handle_->param(configuration_namespace + "tracking_matchers/pose_tracking_recovery_minimum_number_of_failed_registrations_since_last_valid_pose", pose_tracking_recovery_minimum_number_of_failed_registrations_since_last_valid_pose_, 3);
	private_node_handle_->param(configuration_namespace + "tracking_matchers/pose_tracking_recovery_maximum_number_of_failed_registrations_since_last_valid_pose", pose_tracking_recovery_maximum_number_of_failed_registrations_since_last_valid_pose_, 5);
}


template<typename PointT>
void Localization<PointT>::setupInitialPoseEstimatorsFeatureMatchers(const std::string& configuration_namespace) {
	ROS_DEBUG_STREAM("Loading [initial_pose_estimators_feature_matchers] configurations from parameter server namespace [" << (configuration_namespace.empty() ? "~" : configuration_namespace) << "]");
	initial_pose_estimators_feature_matchers_.clear();
	setupFeatureCloudMatchersConfigurations(initial_pose_estimators_feature_matchers_, configuration_namespace + "initial_pose_estimators_matchers/feature_matchers/");
}


template<typename PointT>
void Localization<PointT>::setupInitialPoseEstimatorsPointMatchers(const std::string& configuration_namespace) {
	ROS_DEBUG_STREAM("Loading [initial_pose_estimators_point_matchers] configurations from parameter server namespace [" << (configuration_namespace.empty() ? "~" : configuration_namespace) << "]");
	initial_pose_estimators_point_matchers_.clear();
	setupPointCloudMatchersConfigurations(initial_pose_estimators_point_matchers_, configuration_namespace + "initial_pose_estimators_matchers/point_matchers/");
}


template<typename PointT>
void Localization<PointT>::setupTrackingMatchers(const std::string& configuration_namespace) {
	ROS_DEBUG_STREAM("Loading [tracking_matchers] configurations from parameter server namespace [" << (configuration_namespace.empty() ? "~" : configuration_namespace) << "]");
	tracking_matchers_.clear();
	setupFeatureCloudMatchersConfigurations(tracking_matchers_, "tracking_matchers/feature_matchers/");
	setupPointCloudMatchersConfigurations(tracking_matchers_, "tracking_matchers/point_matchers/");
}


template<typename PointT>
void Localization<PointT>::setupTrackingRecoveryMatchers(const std::string& configuration_namespace) {
	ROS_DEBUG_STREAM("Loading [tracking_recovery_matchers] configurations from parameter server namespace [" << (configuration_namespace.empty() ? "~" : configuration_namespace) << "]");
	tracking_recovery_matchers_.clear();
	setupFeatureCloudMatchersConfigurations(tracking_recovery_matchers_, "tracking_recovery_matchers/feature_matchers/");
	setupPointCloudMatchersConfigurations(tracking_recovery_matchers_, "tracking_recovery_matchers/point_matchers/");
}


template<typename PointT>
void Localization<PointT>::setupPointCloudMatchersConfigurations(std::vector< typename CloudMatcher<PointT>::Ptr >& pointcloud_matchers, const std::string& configuration_namespace) {
	XmlRpc::XmlRpcValue matchers;
	if (private_node_handle_->getParam(configuration_namespace, matchers) && matchers.getType() == XmlRpc::XmlRpcValue::TypeStruct) {
		for (XmlRpc::XmlRpcValue::iterator it = matchers.begin(); it != matchers.end(); ++it) {
			std::string matcher_name = it->first;
			typename CloudMatcher<PointT>::Ptr cloud_matcher;
			if (matcher_name.find("iterative_closest_point_generalized") != std::string::npos) {
				cloud_matcher.reset(new IterativeClosestPointGeneralized<PointT>());
			} else if (matcher_name.find("iterative_closest_point_with_normals") != std::string::npos) {
				cloud_matcher.reset(new IterativeClosestPointWithNormals<PointT>());
			} else if (matcher_name.find("iterative_closest_point_non_linear") != std::string::npos) {
				cloud_matcher.reset(new IterativeClosestPointNonLinear<PointT>());
			} else if (matcher_name.find("iterative_closest_point_2d") != std::string::npos) {
				cloud_matcher.reset(new IterativeClosestPoint2D<PointT>());
			} else if (matcher_name.find("iterative_closest_point") != std::string::npos) {
				cloud_matcher.reset(new IterativeClosestPoint<PointT>());
			} else if (matcher_name.find("normal_distributions_transform_2d") != std::string::npos) {
				cloud_matcher.reset(new NormalDistributionsTransform2D<PointT>());
			} else if (matcher_name.find("normal_distributions_transform_3d") != std::string::npos) {
				cloud_matcher.reset(new NormalDistributionsTransform3D<PointT>());
			} else if (matcher_name.find("principal_component_analysis") != std::string::npos) {
				cloud_matcher.reset(new PrincipalComponentAnalysis<PointT>());
			}

			if (cloud_matcher) {
				cloud_matcher->setupConfigurationFromParameterServer(node_handle_, private_node_handle_, configuration_namespace + matcher_name + "/");
				pointcloud_matchers.push_back(cloud_matcher);
			}
		}
	}
}


template<typename PointT>
void Localization<PointT>::setupFeatureCloudMatchersConfigurations(std::vector< typename CloudMatcher<PointT>::Ptr >& featurecloud_matchers, const std::string& configuration_namespace) {
	std::string keypoint_descriptor_configuration_namespace(configuration_namespace + "keypoint_descriptors/");
	std::string feature_matcher_configuration_namespace(configuration_namespace + "matchers/");
	XmlRpc::XmlRpcValue keypoint_descriptors;
	if (private_node_handle_->getParam(keypoint_descriptor_configuration_namespace, keypoint_descriptors) && keypoint_descriptors.getType() == XmlRpc::XmlRpcValue::TypeStruct) {
		for (XmlRpc::XmlRpcValue::iterator it = keypoint_descriptors.begin(); it != keypoint_descriptors.end(); ++it) {
			std::string descriptor_name = it->first;
			if (descriptor_name.find("fpfh") != std::string::npos) {
				typename KeypointDescriptor<PointT, pcl::FPFHSignature33>::Ptr keypoint_descriptor(new FPFH<PointT, pcl::FPFHSignature33>());
				loadKeypointMatcherFromParameterServer<pcl::FPFHSignature33>(featurecloud_matchers, keypoint_descriptor, keypoint_descriptor_configuration_namespace + descriptor_name + "/", feature_matcher_configuration_namespace);
				return;
			} else if (descriptor_name.find("pfh") != std::string::npos) {
				typename KeypointDescriptor<PointT, pcl::PFHSignature125>::Ptr keypoint_descriptor(new PFH<PointT, pcl::PFHSignature125>());
				loadKeypointMatcherFromParameterServer<pcl::PFHSignature125>(featurecloud_matchers, keypoint_descriptor, keypoint_descriptor_configuration_namespace + descriptor_name + "/", feature_matcher_configuration_namespace);
				return;
			} else if (descriptor_name.find("shot") != std::string::npos) {
				typename KeypointDescriptor<PointT, pcl::SHOT352>::Ptr keypoint_descriptor(new SHOT<PointT, pcl::SHOT352>());
				loadKeypointMatcherFromParameterServer<pcl::SHOT352>(featurecloud_matchers, keypoint_descriptor, keypoint_descriptor_configuration_namespace + descriptor_name + "/", feature_matcher_configuration_namespace);
				return;
			} else if (descriptor_name.find("shape_context_3d") != std::string::npos) {
				typename KeypointDescriptor<PointT, pcl::ShapeContext1980>::Ptr keypoint_descriptor(new ShapeContext3D<PointT, pcl::ShapeContext1980>());
				loadKeypointMatcherFromParameterServer<pcl::ShapeContext1980>(featurecloud_matchers, keypoint_descriptor, keypoint_descriptor_configuration_namespace + descriptor_name + "/", feature_matcher_configuration_namespace);
				return;
			} else if (descriptor_name.find("unique_shape_context") != std::string::npos) {
				typename KeypointDescriptor<PointT, pcl::ShapeContext1980>::Ptr keypoint_descriptor(new UniqueShapeContext<PointT, pcl::ShapeContext1980>());
				loadKeypointMatcherFromParameterServer<pcl::ShapeContext1980>(featurecloud_matchers, keypoint_descriptor, keypoint_descriptor_configuration_namespace + descriptor_name + "/", feature_matcher_configuration_namespace);
				return;
			}/* else if (descriptor_name.find("spin_image") != std::string::npos) {
				typename KeypointDescriptor<PointT, pcl::Histogram<153> >::Ptr keypoint_descriptor(new SpinImage< PointT, pcl::Histogram<153> >());
				loadKeypointMatcherFromParameterServer< pcl::Histogram<153> >(featurecloud_matchers, keypoint_descriptor, keypoint_descriptor_configuration_namespace + descriptor_name + "/", feature_matcher_configuration_namespace);
				return;
			}*/ else if (descriptor_name.find("esf") != std::string::npos) {
				typename KeypointDescriptor<PointT, pcl::ESFSignature640>::Ptr keypoint_descriptor(new ESF<PointT, pcl::ESFSignature640>());
				loadKeypointMatcherFromParameterServer<pcl::ESFSignature640>(featurecloud_matchers, keypoint_descriptor, keypoint_descriptor_configuration_namespace + descriptor_name + "/", feature_matcher_configuration_namespace);
				return;
			}
		}
	}
}


template<typename PointT>
template<typename DescriptorT>
void Localization<PointT>::loadKeypointMatcherFromParameterServer(std::vector< typename CloudMatcher<PointT>::Ptr >& featurecloud_matchers, typename KeypointDescriptor<PointT, DescriptorT>::Ptr& keypoint_descriptor,
		const std::string& keypoint_descriptor_configuration_namespace, const std::string& feature_matcher_configuration_namespace) {
	keypoint_descriptor->setupConfigurationFromParameterServer(node_handle_, private_node_handle_, keypoint_descriptor_configuration_namespace);

	XmlRpc::XmlRpcValue keypoint_matchers;
	if (private_node_handle_->getParam(feature_matcher_configuration_namespace, keypoint_matchers) && keypoint_matchers.getType() == XmlRpc::XmlRpcValue::TypeStruct) {
		for (XmlRpc::XmlRpcValue::iterator it = keypoint_matchers.begin(); it != keypoint_matchers.end(); ++it) {
			std::string matcher_name = it->first;
			if (matcher_name.find("sample_consensus_initial_alignment_prerejective") != std::string::npos) {
				typename FeatureMatcher<PointT, DescriptorT>::Ptr initial_aligment_matcher(new SampleConsensusInitialAlignmentPrerejective<PointT, DescriptorT>());
				initial_aligment_matcher->setKeypointDescriptor(keypoint_descriptor);
				initial_aligment_matcher->setupConfigurationFromParameterServer(node_handle_, private_node_handle_, feature_matcher_configuration_namespace + matcher_name + "/");
				featurecloud_matchers.push_back(initial_aligment_matcher);
			} else if (matcher_name.find("sample_consensus_initial_alignment") != std::string::npos) {
				typename FeatureMatcher<PointT, DescriptorT>::Ptr initial_aligment_matcher(new SampleConsensusInitialAlignment<PointT, DescriptorT>());
				initial_aligment_matcher->setKeypointDescriptor(keypoint_descriptor);
				initial_aligment_matcher->setupConfigurationFromParameterServer(node_handle_, private_node_handle_, feature_matcher_configuration_namespace + matcher_name + "/");
				featurecloud_matchers.push_back(initial_aligment_matcher);
			}
		}
	}
}


template<typename PointT>
void Localization<PointT>::setupTransformationValidatorsConfigurations(std::vector< TransformationValidator::Ptr >& validators, const std::string& configuration_namespace) {
	validators.clear();
	XmlRpc::XmlRpcValue transformation_validators;
	if (private_node_handle_->getParam(configuration_namespace, transformation_validators) && transformation_validators.getType() == XmlRpc::XmlRpcValue::TypeStruct) {
		for (XmlRpc::XmlRpcValue::iterator it = transformation_validators.begin(); it != transformation_validators.end(); ++it) {
			std::string validator_name = it->first;
			TransformationValidator::Ptr transformation_validator;
			if (validator_name.find("euclidean_transformation_validator") != std::string::npos) {
				transformation_validator.reset(new EuclideanTransformationValidator());
			}

			if (transformation_validator) {
				transformation_validator->setupConfigurationFromParameterServer(node_handle_, private_node_handle_, configuration_namespace + validator_name + "/");
				validators.push_back(transformation_validator);
			}
		}
	}
}


template<typename PointT>
void Localization<PointT>::setupTransformationValidatorsForInitialAlignment(const std::string& configuration_namespace) {
	ROS_DEBUG_STREAM("Loading [transformation_validators_for_initial_alignment] configurations from parameter server namespace [" << (configuration_namespace.empty() ? "~" : configuration_namespace) << "]");
	setupTransformationValidatorsConfigurations(transformation_validators_initial_alignment_, "transformation_validators_initial_alignment/");
}


template<typename PointT>
void Localization<PointT>::setupTransformationValidatorsForTracking(const std::string& configuration_namespace) {
	ROS_DEBUG_STREAM("Loading [transformation_validators_for_tracking] configurations from parameter server namespace [" << (configuration_namespace.empty() ? "~" : configuration_namespace) << "]");
	setupTransformationValidatorsConfigurations(transformation_validators_, "transformation_validators/");
}


template<typename PointT>
void Localization<PointT>::setupTransformationValidatorsForTrackingRecovery(const std::string& configuration_namespace) {
	ROS_DEBUG_STREAM("Loading [transformation_validators_for_tracking_recovery] configurations from parameter server namespace [" << (configuration_namespace.empty() ? "~" : configuration_namespace) << "]");
	setupTransformationValidatorsConfigurations(transformation_validators_tracking_recovery_, "transformation_validators_tracking_recovery/");
}


template<typename PointT>
void Localization<PointT>::setupOutlierDetectorsConfigurations(const std::string& configuration_namespace) {
	setupOutlierDetectorsConfigurations(outlier_detectors_, configuration_namespace + "outlier_detectors/", "aligned_");
}


template<typename PointT>
void Localization<PointT>::setupOutlierDetectorsConfigurationsReferencePointCloud(const std::string& configuration_namespace) {
	setupOutlierDetectorsConfigurations(outlier_detectors_reference_pointcloud_, configuration_namespace + "outlier_detectors_reference_pointcloud/", "reference_");
}


template<typename PointT>
void Localization<PointT>::setupOutlierDetectorsConfigurations(std::vector< typename OutlierDetector<PointT>::Ptr >& outlier_detectors, const std::string& configuration_namespace_detectors, const std::string& topics_configuration_prefix) {
	ROS_DEBUG_STREAM("Loading [outlier_detectors] configurations from parameter server namespace [" << (configuration_namespace_detectors.empty() ? "~" : configuration_namespace_detectors) << "]");
	outlier_detectors.clear();
	XmlRpc::XmlRpcValue outlier_detectors_rpc;
	if (private_node_handle_->getParam(configuration_namespace_detectors, outlier_detectors_rpc) && outlier_detectors_rpc.getType() == XmlRpc::XmlRpcValue::TypeStruct) {
		for (XmlRpc::XmlRpcValue::iterator it = outlier_detectors_rpc.begin(); it != outlier_detectors_rpc.end(); ++it) {
			std::string detector_name = it->first;
			typename OutlierDetector<PointT>::Ptr outlier_detector;
			if (detector_name.find("euclidean_outlier_detector") != std::string::npos) {
				outlier_detector.reset(new EuclideanOutlierDetector<PointT>(topics_configuration_prefix));
			}

			if (outlier_detector) {
				outlier_detector->setupConfigurationFromParameterServer(node_handle_, private_node_handle_, configuration_namespace_detectors + detector_name + "/");
				outlier_detectors.push_back(outlier_detector);
			}
		}
	}
}


template<typename PointT>
void Localization<PointT>::setupCloudAnalyzersConfigurations(const std::string& configuration_namespace) {
	ROS_DEBUG_STREAM("Loading [cloud_analyzers] configurations from parameter server namespace [" << (configuration_namespace.empty() ? "~" : configuration_namespace) << "]");
	std::string configuration_namespace_analyzers = configuration_namespace + "cloud_analyzers/";
	private_node_handle_->param(configuration_namespace_analyzers + "compute_inliers_angular_distribution", compute_inliers_angular_distribution_, true);
	private_node_handle_->param(configuration_namespace_analyzers + "compute_outliers_angular_distribution", compute_outliers_angular_distribution_, true);

	cloud_analyzer_.reset();
	XmlRpc::XmlRpcValue cloud_analyzers;
	if (private_node_handle_->getParam(configuration_namespace_analyzers, cloud_analyzers) && cloud_analyzers.getType() == XmlRpc::XmlRpcValue::TypeStruct) {
		for (XmlRpc::XmlRpcValue::iterator it = cloud_analyzers.begin(); it != cloud_analyzers.end(); ++it) {
			std::string cloud_analyzer_name = it->first;
			if (cloud_analyzer_name.find("angular_distribution_analyzer") != std::string::npos) {
				cloud_analyzer_.reset(new AngularDistributionAnalyzer<PointT>());
			}

			if (cloud_analyzer_) {
				cloud_analyzer_->setupConfigurationFromParameterServer(node_handle_, private_node_handle_, configuration_namespace_analyzers + cloud_analyzer_name + "/");
			}
		}
	}
}


template<typename PointT>
void Localization<PointT>::setupRegistrationCovarianceEstimatorsConfigurations(const std::string& configuration_namespace) {
	ROS_DEBUG_STREAM("Loading [registration_covariance_estimators] configurations from parameter server namespace [" << (configuration_namespace.empty() ? "~" : configuration_namespace) << "]");
	std::string configuration_namespace_estimators = configuration_namespace + "registration_covariance_estimator/";

	registration_covariance_estimator_.reset();
	std::string covariance_error_metric;
	private_node_handle_->param(configuration_namespace_estimators + "error_metric", covariance_error_metric, std::string("None"));

	if (covariance_error_metric == "PointToPointPM3D") {
		registration_covariance_estimator_.reset(new RegistrationCovariancePointToPointPM3D<PointT>());
	} else if (covariance_error_metric == "PointToPlanePM3D") {
		registration_covariance_estimator_.reset(new RegistrationCovariancePointToPlanePM3D<PointT>());
	} else if (covariance_error_metric == "PointToPoint3D") {
		registration_covariance_estimator_.reset(new RegistrationCovariancePointToPoint3D<PointT>());
	} else if (covariance_error_metric == "PointToPlane3D") {
		registration_covariance_estimator_.reset(new RegistrationCovariancePointToPlane3D<PointT>());
	} else {
		return;
	}

	if (registration_covariance_estimator_)
		registration_covariance_estimator_->setupConfigurationFromParameterServer(node_handle_, private_node_handle_, configuration_namespace_estimators);
}


template<typename PointT>
void Localization<PointT>::setupTransformationAlignerConfigurations(const std::string &configuration_namespace) {
	ROS_DEBUG_STREAM("Loading [transformation_aligner] configurations from parameter server namespace [" << (configuration_namespace.empty() ? "~" : configuration_namespace) << "]");
	std::string configuration_namespace_aligner = configuration_namespace + "transformation_aligner/";

	transformation_aligner_.reset();
	XmlRpc::XmlRpcValue aligner_values;
	if (private_node_handle_->hasParam(configuration_namespace_aligner) && private_node_handle_->getParam(configuration_namespace_aligner, aligner_values)) {
		if (aligner_values.getType() == XmlRpc::XmlRpcValue::TypeStruct) {
			transformation_aligner_.reset(new TransformationAligner());
			transformation_aligner_->setupConfigurationFromParameterServer(node_handle_, private_node_handle_, configuration_namespace_aligner);
		}
	}
}


template<typename PointT>
bool Localization<PointT>::loadReferencePointCloud() {
	if (reference_pointcloud_required_ && reference_pointcloud_available_ && !reference_pointcloud_filename_.empty()) {
		if (!loadReferencePointCloudFromFile(reference_pointcloud_filename_, reference_pointclouds_database_folder_path_)) {
			ROS_ERROR("Reference point cloud topic or file for localization system must be provided!");
			return false;
		}
	}
	return true;
}


template<typename PointT>
bool Localization<PointT>::loadReferencePointCloudFromFile(const std::string& reference_pointcloud_filename, const std::string& reference_pointclouds_database_folder_path) {
	PerformanceTimer performance_timer;
	performance_timer.start();
	if (pointcloud_conversions::fromFile(*reference_pointcloud_, reference_pointcloud_filename, (reference_pointclouds_database_folder_path.empty() ? reference_pointclouds_database_folder_path_ : reference_pointclouds_database_folder_path))) {
		if (reference_pointcloud_->size() > (size_t)minimum_number_of_points_in_reference_pointcloud_) {
			ROS_INFO_STREAM("Loaded reference point cloud from file " << reference_pointcloud_filename << " with " << reference_pointcloud_->size() << " points in " << performance_timer.getElapsedTimeFormated());
			reference_pointcloud_->header.frame_id = map_frame_id_for_publishing_pointclouds_;

			last_map_received_time_ = ros::Time::now();
			if (reference_cloud_normal_estimator_) reference_cloud_normal_estimator_->resetOccupancyGridMsg();
			return updateLocalizationPipelineWithNewReferenceCloud(ros::Time::now());
		}
	}

	ROS_WARN_STREAM("Failed to loaded reference point cloud from file " << reference_pointcloud_filename);
	reference_pointcloud_loaded_ = false;
	return false;
}


template<typename PointT>
void Localization<PointT>::loadReferencePointCloudFromROSPointCloud(const sensor_msgs::PointCloud2ConstPtr& reference_pointcloud_msg) {
	PerformanceTimer performance_timer;
	performance_timer.start();
	if ((reference_pointcloud_msg->width * reference_pointcloud_msg->height > (size_t)minimum_number_of_points_in_reference_pointcloud_) && (!reference_pointcloud_loaded_ || (ros::Time::now() - last_map_received_time_) > min_seconds_between_reference_pointcloud_update_)) {
		if (reference_pointcloud_msg->width > 0 && reference_pointcloud_msg->data.size() > 0 && reference_pointcloud_msg->fields.size() >= 3) {
			pcl::fromROSMsg(*reference_pointcloud_msg, *reference_pointcloud_);
			size_t pointcloud_size = reference_pointcloud_->size();

			std::vector<int> indexes;
			pcl::removeNaNFromPointCloud(*reference_pointcloud_, *reference_pointcloud_, indexes);
			indexes.clear();

			size_t number_of_nans_in_reference_pointcloud = pointcloud_size - reference_pointcloud_->size();
			if (number_of_nans_in_reference_pointcloud > 0) {
				ROS_DEBUG_STREAM("Removed " << number_of_nans_in_reference_pointcloud << " NaNs from reference cloud with " << pointcloud_size << " points");
			}

			if (reference_pointcloud_->size() > (size_t)minimum_number_of_points_in_reference_pointcloud_) {
				if (reference_pointcloud_msg->header.frame_id != map_frame_id_ && !transformCloudToTFFrame(reference_pointcloud_, reference_pointcloud_msg->header.stamp, map_frame_id_for_transforming_pointclouds_)) { return; }
				if (reference_pointcloud_2d_) { resetPointCloudHeight(*reference_pointcloud_); }
				if (reference_cloud_normal_estimator_) reference_cloud_normal_estimator_->resetOccupancyGridMsg();
				if (updateLocalizationPipelineWithNewReferenceCloud(reference_pointcloud_msg->header.stamp)) {
					ROS_INFO_STREAM("Loaded reference point cloud from cloud topic " << reference_pointcloud_topic_ << " with " << reference_pointcloud_->size() << " points in " << performance_timer.getElapsedTimeFormated());
					last_map_received_time_ = ros::Time::now();
				} else {
					reference_pointcloud_loaded_ = false;
					ROS_WARN_STREAM("Failed to load reference point cloud from cloud topic " << reference_pointcloud_topic_);
				}
			} else {
				ROS_WARN_STREAM("Failed to load reference point cloud from cloud topic " << reference_pointcloud_topic_);
				reference_pointcloud_loaded_ = false;
			}
		}
	}
}


template<typename PointT>
void Localization<PointT>::loadReferencePointCloudFromROSOccupancyGrid(const nav_msgs::OccupancyGridConstPtr& occupancy_grid_msg) {
	PerformanceTimer performance_timer;
	performance_timer.start();
	size_t number_points_in_occupancy_grid = occupancy_grid_msg->info.width * occupancy_grid_msg->info.height;
	if (number_points_in_occupancy_grid > (size_t)minimum_number_of_points_in_reference_pointcloud_ && (!reference_pointcloud_loaded_ || (ros::Time::now() - last_map_received_time_) > min_seconds_between_reference_pointcloud_update_)) {
		typename pcl::PointCloud<PointT>::Ptr reference_pointcloud_from_occupancy_grid(new pcl::PointCloud<PointT>());
		if (pointcloud_conversions::fromROSMsg(*occupancy_grid_msg, *reference_pointcloud_from_occupancy_grid)) {
			if (reference_pointcloud_from_occupancy_grid->size() > (size_t)minimum_number_of_points_in_reference_pointcloud_) {
				reference_pointcloud_2d_ = true;
				if (occupancy_grid_msg->header.frame_id != map_frame_id_ && !transformCloudToTFFrame(reference_pointcloud_from_occupancy_grid, occupancy_grid_msg->header.stamp, map_frame_id_for_transforming_pointclouds_)) { return; }
				reference_pointcloud_ = reference_pointcloud_from_occupancy_grid;
				reference_pointcloud_->header.frame_id = map_frame_id_for_publishing_pointclouds_;
				if (flip_normals_using_occupancy_grid_analysis_ && reference_cloud_normal_estimator_) reference_cloud_normal_estimator_->setOccupancyGridMsg(occupancy_grid_msg);
				if (updateLocalizationPipelineWithNewReferenceCloud(occupancy_grid_msg->header.stamp)) {
					ROS_INFO_STREAM("Loaded reference point cloud from costmap topic " << reference_costmap_topic_ << " with " << reference_pointcloud_->size() << " points in " << performance_timer.getElapsedTimeFormated());
					last_map_received_time_ = ros::Time::now();
					return;
				} else {
					reference_pointcloud_loaded_ = false;
				}
			}
		}
		ROS_WARN_STREAM("Failed to load reference point cloud from nav_msgs/OccupancyGrid msg " << reference_costmap_topic_ << " with " << number_points_in_occupancy_grid << " cells (" << reference_pointcloud_from_occupancy_grid->size() << " of which were occupied cells)");
	}
}


template<typename PointT>
void Localization<PointT>::publishReferencePointCloud(const ros::Time& time_stamp, bool update_msg) {
	if (!reference_pointcloud_publisher_.getTopic().empty()) {
		if (!reference_pointcloud_msg_ || update_msg) {
			reference_pointcloud_msg_ = sensor_msgs::PointCloud2Ptr(new sensor_msgs::PointCloud2());
			pcl::toROSMsg(*reference_pointcloud_, *reference_pointcloud_msg_);
		}

		reference_pointcloud_msg_->header.frame_id = reference_pointcloud_->header.frame_id;
		reference_pointcloud_msg_->header.stamp = time_stamp;
		++reference_pointcloud_msg_->header.seq;
		reference_pointcloud_publisher_.publish(reference_pointcloud_msg_);
	}

	if (!reference_pointcloud_keypoints_publisher_.getTopic().empty()) {
		if (!reference_pointcloud_keypoints_msg_ || update_msg) {
			reference_pointcloud_keypoints_msg_ = sensor_msgs::PointCloud2Ptr(new sensor_msgs::PointCloud2());
			pcl::toROSMsg(*reference_pointcloud_keypoints_, *reference_pointcloud_keypoints_msg_);
		}

		reference_pointcloud_keypoints_msg_->header.frame_id = reference_pointcloud_->header.frame_id;
		reference_pointcloud_keypoints_msg_->header.stamp = time_stamp;
		++reference_pointcloud_keypoints_msg_->header.seq;
		reference_pointcloud_keypoints_publisher_.publish(reference_pointcloud_keypoints_msg_);
	}
}


template<typename PointT>
bool Localization<PointT>::updateLocalizationPipelineWithNewReferenceCloud(const ros::Time& time_stamp) {
	reference_pointcloud_->header.stamp = pcl_conversions::toPCL(time_stamp);
	localization_diagnostics_msg_.number_points_reference_pointcloud = reference_pointcloud_->size();

	std::vector<int> indexes;
	pcl::removeNaNFromPointCloud(*reference_pointcloud_, *reference_pointcloud_, indexes);
	indexes.clear();
	pcl::removeNaNNormalsFromPointCloud(*reference_pointcloud_, *reference_pointcloud_, indexes);
	indexes.clear();

	typename pcl::PointCloud<PointT>::Ptr reference_pointcloud_raw;
	if (!use_filtered_cloud_as_normal_estimation_surface_reference_) {
		reference_pointcloud_raw = typename pcl::PointCloud<PointT>::Ptr(new pcl::PointCloud<PointT>(*reference_pointcloud_));
	}

	if (!applyFilters(reference_cloud_filters_, reference_pointcloud_)) { return false; }
	localization_diagnostics_msg_.number_points_reference_pointcloud_after_filtering = reference_pointcloud_->size();

	if (reference_pointcloud_->size() > (size_t)minimum_number_of_points_in_reference_pointcloud_) {
		reference_pointcloud_search_method_->setInputCloud(reference_pointcloud_);
		if (reference_cloud_normal_estimator_ || reference_cloud_curvature_estimator_) {
			if (!applyNormalEstimation(reference_cloud_normal_estimator_, reference_cloud_curvature_estimator_, reference_pointcloud_, reference_pointcloud_raw, reference_pointcloud_search_method_, true)) { return false; }
		}

		if (reference_pointcloud_->size() > (size_t)minimum_number_of_points_in_reference_pointcloud_) {
			if (reference_pointcloud_normalize_normals_) {
				for (size_t i = 0; i < reference_pointcloud_->size(); ++i) {
					(*reference_pointcloud_)[i].getNormalVector3fMap().normalize();
				}
			}

			if (!reference_pointcloud_preprocessed_save_filename_.empty()) {
				ROS_INFO_STREAM("Saving reference pointcloud preprocessed with " << reference_pointcloud_->size() << " points to file " << reference_pointcloud_preprocessed_save_filename_);
				pointcloud_conversions::toFile(reference_pointcloud_preprocessed_save_filename_, *reference_pointcloud_, save_reference_pointclouds_in_binary_format_, reference_pointclouds_database_folder_path_);
			}

			if (!reference_cloud_keypoint_detectors_.empty()) {
				if (reference_pointcloud_keypoints_filename_.empty() || !pointcloud_conversions::fromFile(*reference_pointcloud_keypoints_, reference_pointcloud_keypoints_filename_, reference_pointclouds_database_folder_path_)) {
					applyKeypointDetection(reference_cloud_keypoint_detectors_, reference_pointcloud_, reference_pointcloud_search_method_, reference_pointcloud_keypoints_);

					if (!reference_pointcloud_keypoints_save_filename_.empty()) {
						ROS_INFO_STREAM("Saving reference pointcloud keypoints with " << reference_pointcloud_keypoints_->size() << " points to file " << reference_pointcloud_keypoints_save_filename_);
						pcl::io::savePCDFile<PointT>(reference_pointcloud_keypoints_save_filename_, *reference_pointcloud_keypoints_, save_reference_pointclouds_in_binary_format_);
					}
				} else {
					ROS_INFO_STREAM("Loaded " << reference_pointcloud_keypoints_->size() << " keypoints from file " << reference_pointcloud_keypoints_filename_);
				}
			}
			localization_diagnostics_msg_.number_keypoints_reference_pointcloud = reference_pointcloud_keypoints_->size();

			if (registration_covariance_estimator_) {
				registration_covariance_estimator_->setReferenceCloud(reference_pointcloud_, reference_pointcloud_search_method_);
			}

			updateMatchersReferenceCloud();
			publishReferencePointCloud(time_stamp, true);
			reference_pointcloud_loaded_ = true;
			return true;
		}
	}

	reference_pointcloud_loaded_ = false;
	return false;
}


template<typename PointT>
void Localization<PointT>::updateMatchersReferenceCloud() {
	ROS_INFO("Updating matchers reference point cloud");

	for (size_t i = 0; i < initial_pose_estimators_feature_matchers_.size(); ++i) {
		initial_pose_estimators_feature_matchers_[i]->setupReferenceCloud(reference_pointcloud_, reference_pointcloud_keypoints_, reference_pointcloud_search_method_);
	}

	for (size_t i = 0; i < initial_pose_estimators_point_matchers_.size(); ++i) {
		initial_pose_estimators_point_matchers_[i]->setupReferenceCloud(reference_pointcloud_, reference_pointcloud_keypoints_, reference_pointcloud_search_method_);
	}

	for (size_t i = 0; i < tracking_matchers_.size(); ++i) {
		tracking_matchers_[i]->setupReferenceCloud(reference_pointcloud_, reference_pointcloud_keypoints_, reference_pointcloud_search_method_);
	}

	for (size_t i = 0; i < tracking_recovery_matchers_.size(); ++i) {
		tracking_recovery_matchers_[i]->setupReferenceCloud(reference_pointcloud_, reference_pointcloud_keypoints_, reference_pointcloud_search_method_);
	}

	ROS_INFO("Finished updating matchers reference point cloud");
}


template<typename PointT>
void Localization<PointT>::setInitialPose(const geometry_msgs::Pose& pose, const std::string& frame_id, const ros::Time& pose_time) {
	ros::Time pose_time_updated = pose_time;
	if ((pose_time.sec == 0 && pose_time.nsec == 0) || pose_time.toSec() < 3.0) {
		pose_time_updated = ros::Time::now();
	}

	if (!initial_pose_msg_needs_to_be_in_map_frame_ || frame_id == map_frame_id_) {
		tf2::Vector3 transform_base_link_to_map_position(pose.position.x, pose.position.y, pose.position.z);
		tf2::Quaternion transform_base_link_to_map_orientation(pose.orientation.x, pose.orientation.y, pose.orientation.z, pose.orientation.w);
		transform_base_link_to_map_orientation.normalize();
		tf2::Transform transform_base_link_to_map(transform_base_link_to_map_orientation, transform_base_link_to_map_position);

		if (!math_utils::isTransformValid(transform_base_link_to_map)) {
			ROS_WARN("Discarded initial pose with NaN values!");
			return;
		}

		if (invert_initial_poses_from_msgs_)
			transform_base_link_to_map = transform_base_link_to_map.inverse();

		bool tf_available = false;
		tf2::Transform transform_odom_to_base_link;
		if (pose_to_tf_publisher_->getTfCollector().lookForTransform(transform_odom_to_base_link, base_link_frame_id_, odom_frame_id_, pose_time, tf_timeout_)) {
			tf_available = true;
		} else {
			if (pose_to_tf_publisher_->getTfCollector().lookForTransform(transform_odom_to_base_link, base_link_frame_id_, odom_frame_id_, ros::Time(0), tf_timeout_)) {
				ROS_WARN_STREAM("Set new initial pose using latest TF because there is no transform from frame [" << base_link_frame_id_ << "] to frame [" << odom_frame_id_ << "] at time " << pose_time);
				tf_available = true;
			}
		}

		if (tf_available) {
			tf2::Transform last_accepted_pose_odom_to_map = transform_base_link_to_map * transform_odom_to_base_link;
			if (!math_utils::isTransformValid(last_accepted_pose_odom_to_map)) {
				ROS_WARN("Discarded initial pose because the multiplication of [transform_base_link_to_map * transform_odom_to_base_link] resulted in a transform with NaN values!");
				return;
			}

			last_accepted_pose_base_link_to_map_ = transform_base_link_to_map;
			last_accepted_pose_odom_to_map_ = last_accepted_pose_odom_to_map;
			last_accepted_pose_time_ = pose_time_updated;
			last_accepted_pose_valid_ = true;
			pose_tracking_number_of_failed_registrations_since_last_valid_pose_ = 0;
			received_external_initial_pose_estimation_ = true;
			ROS_INFO_STREAM("Received initial pose at time [" << pose_time_updated << "]: " \
					<< "\n\tTranslation -> [ x: " << transform_base_link_to_map.getOrigin().getX() << " | y: " << transform_base_link_to_map.getOrigin().getY() << " | z: " << transform_base_link_to_map.getOrigin().getZ() << " ]" \
					<< "\n\tRotation -> [ qx: " << transform_base_link_to_map.getRotation().getX() << " | qy: " << transform_base_link_to_map.getRotation().getY() << " | qz: " << transform_base_link_to_map.getRotation().getZ() << " | qw: " << transform_base_link_to_map.getRotation().getW() << " ]");
		} else {
			ROS_WARN_STREAM("Discarded initial pose because there is no TF from frame [" << base_link_frame_id_ << "] to frame [" << odom_frame_id_ << "]");
		}
	} else {
		ROS_WARN_STREAM("Discarded initial pose because it should be in [" << map_frame_id_ << "] frame instead of [" << frame_id << "] frame");
	}
}


template<typename PointT>
void Localization<PointT>::setInitialPoseFromPose(const geometry_msgs::PoseConstPtr& pose) {
	setInitialPose(*pose, map_frame_id_, ros::Time::now());
}


template<typename PointT>
void Localization<PointT>::setInitialPoseFromPoseStamped(const geometry_msgs::PoseStampedConstPtr& pose) {
	setInitialPose(pose->pose, pose->header.frame_id, pose->header.stamp);
}


template<typename PointT>
void Localization<PointT>::setInitialPoseFromPoseWithCovarianceStamped(const geometry_msgs::PoseWithCovarianceStampedConstPtr& pose) {
	setInitialPose(pose->pose.pose, pose->header.frame_id, pose->header.stamp);
}


template<typename PointT>
void Localization<PointT>::startPublishers() {
	if (!reference_pointcloud_publish_topic_.empty())
		reference_pointcloud_publisher_ = node_handle_->advertise<sensor_msgs::PointCloud2>(reference_pointcloud_publish_topic_, 1, true);
	else
		reference_pointcloud_publisher_.shutdown();

	if (!reference_pointcloud_keypoints_publish_topic_.empty())
		reference_pointcloud_keypoints_publisher_ = node_handle_->advertise<sensor_msgs::PointCloud2>(reference_pointcloud_keypoints_publish_topic_, 1, true);
	else
		reference_pointcloud_keypoints_publisher_.shutdown();

	if (!filtered_pointcloud_publish_topic_.empty())
		filtered_pointcloud_publisher_ = node_handle_->advertise<sensor_msgs::PointCloud2>(filtered_pointcloud_publish_topic_, 1, true);
	else
		filtered_pointcloud_publisher_.shutdown();

	if (!aligned_pointcloud_publish_topic_.empty())
		aligned_pointcloud_publisher_ = node_handle_->advertise<sensor_msgs::PointCloud2>(aligned_pointcloud_publish_topic_, 1, true);
	else
		aligned_pointcloud_publisher_.shutdown();

	if (!pose_stamped_publish_topic_.empty())
		pose_stamped_publisher_ = node_handle_->advertise<geometry_msgs::PoseStamped>(pose_stamped_publish_topic_, 5, true);
	else
		pose_stamped_publisher_.shutdown();

	if (!pose_with_covariance_stamped_publish_topic_.empty())
		pose_with_covariance_stamped_publisher_ = node_handle_->advertise<geometry_msgs::PoseWithCovarianceStamped>(pose_with_covariance_stamped_publish_topic_, 5, true);
	else
		pose_with_covariance_stamped_publisher_.shutdown();

	if (!pose_with_covariance_stamped_tracking_reset_publish_topic_.empty())
		pose_with_covariance_stamped_tracking_reset_publisher_ = node_handle_->advertise<geometry_msgs::PoseWithCovarianceStamped>(pose_with_covariance_stamped_tracking_reset_publish_topic_, 5, true);
	else
		pose_with_covariance_stamped_tracking_reset_publisher_.shutdown();

	if (!pose_array_publish_topic_.empty())
		pose_array_publisher_ = node_handle_->advertise<geometry_msgs::PoseArray>(pose_array_publish_topic_, 1, true);
	else
		pose_array_publisher_.shutdown();

	if (!localization_detailed_publish_topic_.empty())
		localization_detailed_publisher_ = node_handle_->advertise<dynamic_robot_localization::LocalizationDetailed>(localization_detailed_publish_topic_, 5, true);
	else
		localization_detailed_publisher_.shutdown();

	if (!localization_diagnostics_publish_topic_.empty())
		localization_diagnostics_publisher_ = node_handle_->advertise<dynamic_robot_localization::LocalizationDiagnostics>(localization_diagnostics_publish_topic_, 5, true);
	else
		localization_diagnostics_publisher_.shutdown();

	if (!localization_times_publish_topic_.empty())
		localization_times_publisher_ = node_handle_->advertise<dynamic_robot_localization::LocalizationTimes>(localization_times_publish_topic_, 5, true);
	else
		localization_times_publisher_.shutdown();
}


template<typename PointT>
void Localization<PointT>::startReferenceCloudSubscribers() {
	if (reference_pointcloud_topic_.empty())
		reference_pointcloud_subscriber_.shutdown();

	if (reference_costmap_topic_.empty())
		costmap_subscriber_.shutdown();

	if (reference_pointcloud_required_ && reference_pointcloud_available_) {
		if (!reference_pointcloud_topic_.empty()) {
			reference_pointcloud_subscriber_ = node_handle_->subscribe(reference_pointcloud_topic_, 1, &dynamic_robot_localization::Localization<PointT>::loadReferencePointCloudFromROSPointCloud, this);
		} else {
			if (!reference_costmap_topic_.empty()) {
				costmap_subscriber_ = node_handle_->subscribe(reference_costmap_topic_, 1, &dynamic_robot_localization::Localization<PointT>::loadReferencePointCloudFromROSOccupancyGrid, this);
			} else if (reference_pointcloud_filename_.empty()) {
				ROS_ERROR("Reference point cloud topic or file for localization system must be provided!");
			}
		}
	}
}


template<typename PointT>
void Localization<PointT>::startSubscribers() {
	if (!pose_topic_.empty())
		pose_subscriber_ = node_handle_->subscribe(pose_topic_, 1, &dynamic_robot_localization::Localization<PointT>::setInitialPoseFromPose, this);
	else
		pose_subscriber_.shutdown();

	if (!pose_stamped_topic_.empty())
		pose_stamped_subscriber_ = node_handle_->subscribe(pose_stamped_topic_, 1, &dynamic_robot_localization::Localization<PointT>::setInitialPoseFromPoseStamped, this);
	else
		pose_stamped_subscriber_.shutdown();

	if (!pose_with_covariance_stamped_topic_.empty())
		pose_with_covariance_stamped_subscriber_ = node_handle_->subscribe(pose_with_covariance_stamped_topic_, 1, &dynamic_robot_localization::Localization<PointT>::setInitialPoseFromPoseWithCovarianceStamped, this);
	else
		pose_with_covariance_stamped_subscriber_.shutdown();

	ambient_pointcloud_subscribers_.clear();
	if (ambient_pointcloud_topics_.empty()) {
		ROS_ERROR("Ambient point cloud topic for localization system must be provided!");
		return;
	} else {
		std::replace(ambient_pointcloud_topics_.begin(), ambient_pointcloud_topics_.end(), '+', ' ');

		std::stringstream ss(ambient_pointcloud_topics_);
		std::string topic_name;

		while (ss >> topic_name && !topic_name.empty()) {
			ambient_pointcloud_subscribers_.push_back(node_handle_->subscribe(topic_name, 1, &dynamic_robot_localization::Localization<PointT>::processAmbientPointCloud, this));
			ROS_INFO_STREAM("Adding " << topic_name << " to the list of sensor_msgs::PointCloud2 topics to use in localization");
		}

		ambient_pointcloud_subscribers_active_ = true;
	}
}


template<typename PointT>
void Localization<PointT>::startServiceServers() {
	if (!reload_localization_configuration_service_server_name_.empty()) {
		reload_localization_configuration_service_server_ = node_handle_->advertiseService(reload_localization_configuration_service_server_name_, &dynamic_robot_localization::Localization<PointT>::reloadConfigurationFromParameterServerServiceCallback, this);
	} else {
		reload_localization_configuration_service_server_.shutdown();
	}
}


template<typename PointT>
void Localization<PointT>::startLocalization(bool start_ros_spinner) {
	if (node_handle_ && private_node_handle_) {
		ROS_DEBUG("Starting self-localization...");

		startPublishers();
		startReferenceCloudSubscribers();

		// initial pose setup might block while waiting for valid TF
		while (!ros::Time::isValid() || (reference_pointcloud_required_ && reference_pointcloud_available_ && !reference_pointcloud_loaded_)) {
			ROS_DEBUG_THROTTLE(1.0, "Waiting for valid time...");
			ros::spinOnce(); // allows to setup reference map before tf is available (which happens when playing bag files with --pause option)
		}
		last_accepted_pose_time_ = ros::Time::now();

		setupInitialPose();
		startSubscribers();
		startServiceServers();

		if (start_ros_spinner) {
			startROSSpinner();
		}
	}
}


template<typename PointT>
void Localization<PointT>::startROSSpinner() {
	if (publish_tf_map_odom_) {
		pose_to_tf_publisher_->startPublishingTF();
	} else {
		ros::spin();
	}
}


template<typename PointT>
void Localization<PointT>::stopProcessingSensorData() {
	ambient_pointcloud_subscribers_active_ = false;
	for (size_t i = 0; i < ambient_pointcloud_subscribers_.size(); ++i) {
		ambient_pointcloud_subscribers_[i].shutdown();
	}
}


template<typename PointT>
void Localization<PointT>::restartProcessingSensorData() {
	resetNumberOfProcessedPointclouds();
	ambient_pointcloud_subscribers_active_ = true;
	sensor_data_processing_status_ = WaitingForSensorData;
	std::vector<std::string> topic_names;

	for (size_t i = 0; i < ambient_pointcloud_subscribers_.size(); ++i) {
		topic_names.push_back(ambient_pointcloud_subscribers_[i].getTopic());
		ambient_pointcloud_subscribers_[i].shutdown();
	}

	ambient_pointcloud_subscribers_.clear();

	for (size_t i = 0; i < topic_names.size(); ++i) {
		ambient_pointcloud_subscribers_.push_back(node_handle_->subscribe(topic_names[i], 1, &dynamic_robot_localization::Localization<PointT>::processAmbientPointCloud, this));
	}
}


template<typename PointT>
void Localization<PointT>::resetNumberOfProcessedPointclouds() {
	number_of_processed_pointclouds_ = 0;
}


template<typename PointT>
bool Localization<PointT>::transformCloudToTFFrame(typename pcl::PointCloud<PointT>::Ptr& ambient_pointcloud, const ros::Time& timestamp, const std::string& target_frame_id) {
	if (ambient_pointcloud->header.frame_id != target_frame_id) {
		tf2::Transform pose_tf_cloud_to_map;
		pose_tf_cloud_to_map.setIdentity();
		bool use_lookup_without_odom = false;
		if ((target_frame_id == map_frame_id_ || target_frame_id == map_frame_id_for_transforming_pointclouds_) && ambient_pointcloud->header.frame_id != odom_frame_id_) {
			if (use_odom_when_transforming_cloud_to_map_frame_) {
				tf2::Transform pose_tf_cloud_to_odom;
				if (!pose_to_tf_publisher_->getTfCollector().lookForTransform(pose_tf_cloud_to_odom, odom_frame_id_, ambient_pointcloud->header.frame_id, timestamp, tf_timeout_)) {
					ROS_WARN_STREAM("Dropping pointcloud because TF [ " << ambient_pointcloud->header.frame_id << " -> " << odom_frame_id_ << " ] was not available");
					return false;
				}
				pose_tf_cloud_to_map = last_accepted_pose_odom_to_map_ * pose_tf_cloud_to_odom;
			} else {
				use_lookup_without_odom = true;
			}
		} else {
			if (target_frame_id == map_frame_id_ || target_frame_id == map_frame_id_for_transforming_pointclouds_) {
				pose_tf_cloud_to_map = last_accepted_pose_odom_to_map_;
			} else {
				use_lookup_without_odom = true;
			}
		}

		if (use_lookup_without_odom) {
			if (!pose_to_tf_publisher_->getTfCollector().lookForTransform(pose_tf_cloud_to_map, target_frame_id, ambient_pointcloud->header.frame_id, timestamp, tf_timeout_)) {
				if (!pose_to_tf_publisher_->getTfCollector().lookForTransform(pose_tf_cloud_to_map, target_frame_id, ambient_pointcloud->header.frame_id, ros::Time(0.0), tf_timeout_)) {
					ROS_WARN_STREAM("Dropping pointcloud because TF [ " << ambient_pointcloud->header.frame_id << " -> " << target_frame_id << " ] was not available");
					return false;
				} else
					ROS_WARN_STREAM("Using TF at Time(0) since at " << timestamp << " [ " << ambient_pointcloud->header.frame_id << " -> " << target_frame_id << " ] was not available");
			}
		}

		if (invert_cloud_to_map_transform_) {
			pose_tf_cloud_to_map = pose_tf_cloud_to_map.inverse();
		}

		if (!math_utils::isTransformValid(pose_tf_cloud_to_map)) {
			ROS_WARN_STREAM("Dropping pointcloud because TF [ " << ambient_pointcloud->header.frame_id << " -> " << odom_frame_id_ << " ] had NaN values");
			return false;
		}

		Eigen::Transform<double, 3, Eigen::Affine> pose_tf_cloud_to_map_eigen_transform = laserscan_to_pointcloud::tf_rosmsg_eigen_conversions::transformTF2ToTransform<double>(pose_tf_cloud_to_map);
		pcl::transformPointCloudWithNormals(*ambient_pointcloud, *ambient_pointcloud, pose_tf_cloud_to_map_eigen_transform);

		std::string transform_string = math_utils::convertTransformToString<double>(pose_tf_cloud_to_map_eigen_transform.matrix());
		ROS_DEBUG_STREAM("Transformed pointcloud with " << ambient_pointcloud->size() << " points from frame " << ambient_pointcloud->header.frame_id << " to frame " << target_frame_id << " using matrix:" << transform_string << "\n");

		if ((target_frame_id == map_frame_id_ || target_frame_id == map_frame_id_for_transforming_pointclouds_) && !map_frame_id_for_publishing_pointclouds_.empty())
			ambient_pointcloud->header.frame_id = map_frame_id_for_publishing_pointclouds_;
		else
			ambient_pointcloud->header.frame_id = target_frame_id;
	}

	return true;
}

template<typename PointT>
bool Localization<PointT>::checkIfAmbientPointCloudShouldBeProcessed(const ros::Time& ambient_cloud_time, size_t number_of_points, bool check_if_pointcloud_subscribers_are_active, bool use_ros_console) {
	bool process_pointcloud = true;
	ros::Duration scan_age = ros::Time::now() - ambient_cloud_time;
	ros::Duration elapsed_time_since_last_scan = last_scan_time_.toNSec() > 0 ? ros::Time::now() - last_scan_time_ : ros::Duration(0.0);
	ros::Duration time_offset = last_scan_time_.toNSec() > 0 ? (last_scan_time_ - ambient_cloud_time) : ros::Duration(0.0);

	if (check_if_pointcloud_subscribers_are_active && !ambient_pointcloud_subscribers_active_) {
		process_pointcloud = false;
		sensor_data_processing_status_ = PointCloudSubscribersDisabled;
		if (use_ros_console)
			ROS_WARN("Discarded point cloud because the subscribers are not active");
	} else if (reference_pointcloud_required_ && reference_pointcloud_available_ &&
				((!reference_pointcloud_loaded_ || reference_pointcloud_->size() < (size_t)minimum_number_of_points_in_reference_pointcloud_) ||
				(!reference_pointcloud_loaded_ && map_update_mode_ == NoIntegration))) {
		process_pointcloud = false;
		sensor_data_processing_status_ = MissingReferencePointCloud;
		if (use_ros_console)
			ROS_WARN_STREAM("Discarded cloud because there is no reference cloud to compare to");
	} else if (number_of_points < (size_t)minimum_number_of_points_in_ambient_pointcloud_) {
		process_pointcloud = false;
		sensor_data_processing_status_ = PointCloudWithoutTheMinimumNumberOfRequiredPoints;
		if (use_ros_console)
			ROS_WARN_STREAM("Discarded ambient cloud [ minimum number of points required: " << minimum_number_of_points_in_ambient_pointcloud_ << " | point cloud size: " << number_of_points << " ]");
	} else if (max_seconds_ambient_pointcloud_offset_to_last_estimated_pose_.toSec() > 0.0 && ambient_cloud_time < last_scan_time_ &&
		std::abs(time_offset.toSec()) > std::abs(max_seconds_ambient_pointcloud_offset_to_last_estimated_pose_.toSec())) {
		process_pointcloud = false;
		sensor_data_processing_status_ = PointCloudOlderThanLastPointCloudReceived;
		if (use_ros_console)
			ROS_WARN_STREAM("Discarded ambient cloud because it's timestamp (" << ambient_cloud_time << ") is " << time_offset.toSec() << " seconds older than an already processed ambient cloud (limit for offset: " << max_seconds_ambient_pointcloud_offset_to_last_estimated_pose_ << ")");
	} else if (min_seconds_between_scan_registration_.toSec() > 0.0 && elapsed_time_since_last_scan < min_seconds_between_scan_registration_) {
		process_pointcloud = false;
		sensor_data_processing_status_ = MinimumElapsedTimeSinceLastPointCloudNotReached;
		if (use_ros_console)
			ROS_WARN_STREAM("Discarded cloud with an elapsed_time_since_last_scan [" << elapsed_time_since_last_scan << "] lower than the minimum allowed " << "[" << min_seconds_between_scan_registration_ << "]");
	} else if (max_seconds_ambient_pointcloud_age_.toSec() > 0 && scan_age > max_seconds_ambient_pointcloud_age_) {
		process_pointcloud = false;
		sensor_data_processing_status_ = PointCloudAgeHigherThanMaximum;
		if (use_ros_console)
			ROS_WARN_STREAM("Discarded cloud with scan_age: [" << scan_age.toSec() << "] higher than the maximum allowed [" << max_seconds_ambient_pointcloud_age_ << "]");
	}

	return process_pointcloud;
}


template<typename PointT>
bool Localization<PointT>::checkIfTrackingIsLost() {
	bool lost_tracking = !robot_initial_pose_available_
			|| ((ros::Time::now() - last_accepted_pose_time_) > pose_tracking_timeout_ && pose_tracking_number_of_failed_registrations_since_last_valid_pose_ > pose_tracking_minimum_number_of_failed_registrations_since_last_valid_pose_)
			|| (pose_tracking_number_of_failed_registrations_since_last_valid_pose_ > pose_tracking_maximum_number_of_failed_registrations_since_last_valid_pose_);

	if (reference_pointcloud_required_ && !reference_pointcloud_available_ && !reference_pointcloud_loaded_ && map_update_mode_ != NoIntegration) { lost_tracking = false; }

	return lost_tracking;
}

template<typename PointT>
void Localization<PointT>::processAmbientPointCloud(const sensor_msgs::PointCloud2ConstPtr& ambient_cloud_msg) {
	ROS_DEBUG_STREAM("Received ROS point cloud message with " << ambient_cloud_msg->width * ambient_cloud_msg->height << " points");

	if (limit_of_pointclouds_to_process_ >= 0 && number_of_processed_pointclouds_ >= (size_t)limit_of_pointclouds_to_process_) {
		ROS_DEBUG("Discarding point cloud because [number_of_processed_pointclouds >= limit_of_pointclouds_to_process]");
		return;
	}

	ros::Time ambient_cloud_time = (override_pointcloud_timestamp_to_current_time_ ? ros::Time::now() : ambient_cloud_msg->header.stamp);
	size_t number_points_ambient_pointcloud = ambient_cloud_msg->width * ambient_cloud_msg->height;

	if (checkIfAmbientPointCloudShouldBeProcessed(ambient_cloud_time, number_points_ambient_pointcloud, true, true))
	{
		typename pcl::PointCloud<PointT>::Ptr ambient_pointcloud(new pcl::PointCloud<PointT>());
		pcl::fromROSMsg(*ambient_cloud_msg, *ambient_pointcloud);
		ambient_pointcloud->header.frame_id = ambient_cloud_msg->header.frame_id;
		processAmbientPointCloud(ambient_pointcloud, false, false);
	}
}

template<typename PointT>
bool Localization<PointT>::processAmbientPointCloud(typename pcl::PointCloud<PointT>::Ptr& ambient_pointcloud, bool check_if_pointcloud_should_be_processed, bool check_if_pointcloud_subscribers_are_active) {
	try {
		if (limit_of_pointclouds_to_process_ >= 0 && number_of_processed_pointclouds_ >= (size_t)limit_of_pointclouds_to_process_) {
			ROS_DEBUG("Discarding point cloud because [number_of_processed_pointclouds >= limit_of_pointclouds_to_process]");
			return false;
		}

		PerformanceTimer performance_timer;
		performance_timer.start();
		localization_times_msg_ = LocalizationTimes();

		ros::Time ambient_cloud_time = (override_pointcloud_timestamp_to_current_time_ ? ros::Time::now() : pcl_conversions::fromPCL(ambient_pointcloud->header.stamp));
		size_t number_points_ambient_pointcloud = ambient_pointcloud->width * ambient_pointcloud->height;
		if (check_if_pointcloud_should_be_processed) {
			if (!checkIfAmbientPointCloudShouldBeProcessed(ambient_cloud_time, number_points_ambient_pointcloud, check_if_pointcloud_subscribers_are_active, true))
				return false;
		}

		if (limit_of_pointclouds_to_process_ > 0) {
			++number_of_processed_pointclouds_;
		}

		ROS_DEBUG_STREAM("Received pointcloud with sequence number " << ambient_pointcloud->header.seq << " in frame " << ambient_pointcloud->header.frame_id << " with " << (ambient_pointcloud->width * ambient_pointcloud->height) << " points and with time stamp " << ambient_cloud_time << " (map_frame_id: " << map_frame_id_ << ")");

		if (checkIfTrackingIsLost()) {
			last_accepted_pose_valid_ = false;
			if (reset_initial_pose_when_tracking_is_lost_) {
				ROS_DEBUG("Resetting initial pose");
				setupInitialPose(configuration_namespace_, ambient_cloud_time);
			}

			if (!tracking_matchers_.empty() || !tracking_recovery_matchers_.empty())
				ROS_ERROR("Lost tracking!");
		}

		tf2::Transform transform_base_link_to_odom;
		if (!pose_to_tf_publisher_->getTfCollector().lookForTransform(transform_base_link_to_odom, odom_frame_id_, base_link_frame_id_, ambient_cloud_time, tf_timeout_) || !math_utils::isTransformValid(transform_base_link_to_odom)) {
			ROS_WARN_STREAM("Dropping pointcloud because tf between " << base_link_frame_id_ << " and " << odom_frame_id_ << " isn't available");
			sensor_data_processing_status_ = FailedTFTransform;
			return false;
		}

		if (!use_internal_tracking_) {
			tf2::Transform transform_odom_to_map;
			if (!pose_to_tf_publisher_->getTfCollector().lookForTransform(transform_odom_to_map, map_frame_id_, odom_frame_id_, ambient_cloud_time, tf_timeout_) || math_utils::isTransformValid(transform_odom_to_map)) {
				ROS_WARN_STREAM("Using internal tracking transform because tf between " << odom_frame_id_ << " and " << map_frame_id_ << " isn't available");
			} else {
				last_accepted_pose_odom_to_map_ = transform_odom_to_map;
			}
		}

		tf2::Transform pose_tf_initial_guess = last_accepted_pose_odom_to_map_ * transform_base_link_to_odom;
		size_t ambient_pointcloud_size = ambient_pointcloud->size();
		std::vector<int> indexes;
		ambient_pointcloud->is_dense = false;
		pcl::removeNaNFromPointCloud(*ambient_pointcloud, *ambient_pointcloud, indexes);
		indexes.clear();
		size_t number_of_nans_in_ambient_pointcloud = ambient_pointcloud_size - ambient_pointcloud->size();
		if (number_of_nans_in_ambient_pointcloud > 0) {
			ROS_DEBUG_STREAM("Removed " << number_of_nans_in_ambient_pointcloud << " NaNs from ambient cloud with " << ambient_pointcloud_size << " points");
		}

		tf2::Quaternion pose_tf_initial_guess_q = pose_tf_initial_guess.getRotation().normalize();
		ROS_DEBUG_STREAM("Initial pose:" \
				<< "\tTF position -> [ x: " << pose_tf_initial_guess.getOrigin().getX() << " | y: " << pose_tf_initial_guess.getOrigin().getY() << " | z: " << pose_tf_initial_guess.getOrigin().getZ() << " ]" \
				<< "\tTF orientation -> [ qx: " << pose_tf_initial_guess_q.getX() << " | qy: " << pose_tf_initial_guess_q.getY() << " | qz: " << pose_tf_initial_guess_q.getZ() << " | qw: " << pose_tf_initial_guess_q.getW() << " ]");


		// >>>>> localization pipeline <<<<<
		tf2::Transform pose_corrections;
		typename pcl::PointCloud<PointT>::Ptr ambient_pointcloud_keypoints(new pcl::PointCloud<PointT>());
		ambient_pointcloud_keypoints->header = ambient_pointcloud->header;

		bool localizationUpdateSuccess = updateLocalizationWithAmbientPointCloud(ambient_pointcloud, ambient_cloud_time, pose_tf_initial_guess, pose_tf2_transform_corrected_, pose_corrections, ambient_pointcloud_keypoints) || (!reference_pointcloud_available_ && !reference_pointcloud_loaded_ && map_update_mode_ != NoIntegration);

		ros::Time pose_time;
		if (add_odometry_displacement_) {
			pose_time = ros::Time::now();
			pose_to_tf_publisher_->addOdometryDisplacementToTransform(pose_tf2_transform_corrected_, ambient_cloud_time, pose_time);
		} else {
			pose_time = ambient_cloud_time;
		}

		tf2::Transform pose_tf_corrected_to_publish = pose_tf2_transform_corrected_;
		if (invert_registration_transformation_) {
			pose_tf_corrected_to_publish = pose_tf2_transform_corrected_.inverse();
			pose_corrections = pose_corrections.inverse();
		}

		geometry_msgs::PoseArrayPtr accepted_poses(new geometry_msgs::PoseArray());
		if (!pose_array_publisher_.getTopic().empty() || !localization_detailed_publisher_.getTopic().empty()) {
			accepted_poses->header.frame_id = use_base_link_frame_when_publishing_initial_poses_array_ ? base_link_frame_id_ : map_frame_id_;
			accepted_poses->header.stamp = pose_time;
			tf2::Transform pose_tf_corrected_to_publish_inverse = pose_tf_corrected_to_publish.inverse();
			for (size_t i = 0; i < accepted_pose_corrections_.size(); ++i) {
				geometry_msgs::Pose accepted_pose;
				tf2::Transform accepted_pose_tf;

				if (apply_cloud_registration_inverse_to_initial_poses_array_)
					accepted_pose_tf = pose_tf_corrected_to_publish_inverse * accepted_pose_corrections_[i] * pose_tf_initial_guess;
				else
					accepted_pose_tf = accepted_pose_corrections_[i] * pose_tf_initial_guess;

				laserscan_to_pointcloud::tf_rosmsg_eigen_conversions::transformTF2ToMsg(accepted_pose_tf, accepted_pose);
				accepted_poses->poses.push_back(accepted_pose);
			}
		}

		if (!pose_array_publisher_.getTopic().empty() && !accepted_poses->poses.empty()) {
			pose_array_publisher_.publish(accepted_poses);
			ROS_INFO_STREAM("Publishing " << accepted_pose_corrections_.size() << " accepted poses");
		}

		if (localizationUpdateSuccess) {
			ambient_pointcloud->header.stamp = (std::uint64_t)(ambient_cloud_time.toNSec() / 1000.0);
			if (republish_reference_pointcloud_after_successful_registration_ && map_update_mode_ == NoIntegration)
				publishReferencePointCloud(ambient_cloud_time, false);

			last_scan_time_ = pose_time;

			if (!localization_times_publisher_.getTopic().empty()) {
				localization_times_msg_.header.frame_id = map_frame_id_;
				localization_times_msg_.header.stamp = ambient_cloud_time;
				localization_times_msg_.global_time = performance_timer.getElapsedTimeInMilliSec();
				localization_times_msg_.correspondence_estimation_time_for_all_matchers = correspondence_estimation_time_for_all_matchers_;
				localization_times_msg_.transformation_estimation_time_for_all_matchers = transformation_estimation_time_for_all_matchers_;
				localization_times_msg_.transform_cloud_time_for_all_matchers = transform_cloud_time_for_all_matchers_;
				localization_times_msg_.cloud_align_time_for_all_matchers = cloud_align_time_for_all_matchers_;
				localization_times_publisher_.publish(localization_times_msg_);
			}

			if (publish_tf_map_odom_) {
				pose_to_tf_publisher_->publishTF(pose_tf_corrected_to_publish, ambient_cloud_time, ambient_cloud_time);
			}

			last_accepted_pose_odom_to_map_ = pose_tf2_transform_corrected_ * transform_base_link_to_odom.inverse();

			tf2::Quaternion pose_tf_corrected_q = pose_tf_corrected_to_publish.getRotation().normalize();
			ROS_DEBUG_STREAM("Corrected pose:" \
					<< "\tTF position -> [ x: " << pose_tf_corrected_to_publish.getOrigin().getX() << " | y: " << pose_tf_corrected_to_publish.getOrigin().getY() << " | z: " << pose_tf_corrected_to_publish.getOrigin().getZ() << " ]" \
					<< "\tTF orientation -> [ qx: " << pose_tf_corrected_q.getX() << " | qy: " << pose_tf_corrected_q.getY() << " | qz: " << pose_tf_corrected_q.getZ() << " | qw: " << pose_tf_corrected_q.getW() << " ]");

			if (!pose_with_covariance_stamped_publisher_.getTopic().empty() || !pose_with_covariance_stamped_tracking_reset_publisher_.getTopic().empty()) {
				geometry_msgs::PoseWithCovarianceStampedPtr pose_corrected_msg(new geometry_msgs::PoseWithCovarianceStamped());
				pose_corrected_msg->header.frame_id = use_base_link_frame_when_publishing_registration_pose_ ? base_link_frame_id_ : map_frame_id_;;
				pose_corrected_msg->header.stamp = pose_time;

				laserscan_to_pointcloud::tf_rosmsg_eigen_conversions::transformTF2ToMsg(pose_tf_corrected_to_publish, pose_corrected_msg->pose.pose);
				fillPoseCovariance(*pose_corrected_msg, last_accepted_pose_covariance_);

				if (!pose_with_covariance_stamped_publisher_.getTopic().empty()) {
					pose_with_covariance_stamped_publisher_.publish(pose_corrected_msg);
				}

				if (last_accepted_pose_performed_tracking_reset_ && !pose_with_covariance_stamped_tracking_reset_publisher_.getTopic().empty()) {
					pose_with_covariance_stamped_tracking_reset_publisher_.publish(pose_corrected_msg);
				}
			}

			if (!pose_stamped_publisher_.getTopic().empty()) {
				geometry_msgs::PoseStampedPtr pose_corrected_msg(new geometry_msgs::PoseStamped());
				pose_corrected_msg->header.frame_id = use_base_link_frame_when_publishing_registration_pose_ ? base_link_frame_id_ : map_frame_id_;
				pose_corrected_msg->header.stamp = pose_time;

				laserscan_to_pointcloud::tf_rosmsg_eigen_conversions::transformTF2ToMsg(pose_tf_corrected_to_publish, pose_corrected_msg->pose);
				pose_stamped_publisher_.publish(pose_corrected_msg);
			}

			if (!localization_detailed_publisher_.getTopic().empty()) {
				LocalizationDetailed localization_detailed_msg;
				localization_detailed_msg.header.frame_id = use_base_link_frame_when_publishing_registration_pose_ ? base_link_frame_id_ : map_frame_id_;
				localization_detailed_msg.header.stamp = ambient_cloud_time;
				laserscan_to_pointcloud::tf_rosmsg_eigen_conversions::transformTF2ToMsg(pose_tf_corrected_to_publish, localization_detailed_msg.pose);
				localization_detailed_msg.outlier_percentage = outlier_percentage_;
				localization_detailed_msg.outlier_percentage_reference_pointcloud = outlier_percentage_reference_pointcloud_;
				localization_detailed_msg.root_mean_square_error_inliers = root_mean_square_error_inliers_;
				localization_detailed_msg.root_mean_square_error_inliers_reference_pointcloud = root_mean_square_error_inliers_reference_pointcloud_;
				if (localization_detailed_use_millimeters_in_root_mean_square_error_inliers_) {
					localization_detailed_msg.root_mean_square_error_inliers *= 1000.0;
					localization_detailed_msg.root_mean_square_error_inliers_reference_pointcloud *= 1000.0;
				}
				localization_detailed_msg.number_inliers = number_inliers_;
				localization_detailed_msg.number_inliers_reference_pointcloud = number_inliers_reference_pointcloud_;
				localization_detailed_msg.number_points_registered = ambient_pointcloud->size();
				localization_detailed_msg.inliers_angular_distribution = inliers_angular_distribution_;
				localization_detailed_msg.outliers_angular_distribution = outliers_angular_distribution_;

				// translation corrections
				if (localization_detailed_compute_pose_corrections_from_initial_and_final_pose_tfs_) {
					localization_detailed_msg.translation_corrections.x = pose_tf_initial_guess.getOrigin().getX() - pose_tf2_transform_corrected_.getOrigin().getX();
					localization_detailed_msg.translation_corrections.y = pose_tf_initial_guess.getOrigin().getY() - pose_tf2_transform_corrected_.getOrigin().getY();
					localization_detailed_msg.translation_corrections.z = pose_tf_initial_guess.getOrigin().getZ() - pose_tf2_transform_corrected_.getOrigin().getZ();
				} else {
					localization_detailed_msg.translation_corrections.x = pose_corrections.getOrigin().getX();
					localization_detailed_msg.translation_corrections.y = pose_corrections.getOrigin().getY();
					localization_detailed_msg.translation_corrections.z = pose_corrections.getOrigin().getZ();
				}

				if (localization_detailed_use_millimeters_in_translation_corrections_) {
					localization_detailed_msg.translation_corrections.x *= 1000.0;
					localization_detailed_msg.translation_corrections.y *= 1000.0;
					localization_detailed_msg.translation_corrections.z *= 1000.0;
				}
				localization_detailed_msg.translation_correction = std::sqrt(
						localization_detailed_msg.translation_corrections.x * localization_detailed_msg.translation_corrections.x +
						localization_detailed_msg.translation_corrections.y * localization_detailed_msg.translation_corrections.y +
						localization_detailed_msg.translation_corrections.z * localization_detailed_msg.translation_corrections.z);

				// rotation corrections
				tf2::Quaternion rotation_corrections;
				if (localization_detailed_compute_pose_corrections_from_initial_and_final_pose_tfs_) {
					rotation_corrections = pose_tf_initial_guess_q * pose_tf_corrected_q.inverse();
				} else {
					rotation_corrections = pose_corrections.getRotation();
				}
				rotation_corrections.normalize();
				if (rotation_corrections.getW() < 0.0) { rotation_corrections *= -1.0; } // shortest path angle
				tf2::Vector3 rotation_correction_axis = rotation_corrections.getAxis().normalize();
				localization_detailed_msg.rotation_correction_angle = rotation_corrections.getAngle();
				localization_detailed_msg.rotation_correction_axis.x = rotation_correction_axis.getX();
				localization_detailed_msg.rotation_correction_axis.y = rotation_correction_axis.getY();
				localization_detailed_msg.rotation_correction_axis.z = rotation_correction_axis.getZ();

				if (localization_detailed_use_degrees_in_rotation_corrections_) {
					localization_detailed_msg.rotation_correction_angle = angles::to_degrees(localization_detailed_msg.rotation_correction_angle);
				}

				if (number_of_registration_iterations_for_all_matchers_ > 0) {
					localization_detailed_msg.number_of_registration_iterations_for_all_matchers = number_of_registration_iterations_for_all_matchers_;
				} else {
					localization_detailed_msg.number_of_registration_iterations_for_all_matchers = -1;
				}

				localization_detailed_msg.last_matcher_convergence_state = last_matcher_convergence_state_;
				localization_detailed_msg.root_mean_square_error_of_last_registration_correspondences = root_mean_square_error_of_last_registration_correspondences_;
				if (localization_detailed_use_millimeters_in_root_mean_square_error_of_last_registration_correspondences_) {
					localization_detailed_msg.root_mean_square_error_of_last_registration_correspondences *= 1000.0;
				}
				localization_detailed_msg.initial_pose_estimation_poses = *accepted_poses;
				localization_detailed_publisher_.publish(localization_detailed_msg);
			}

			if (!localization_diagnostics_publisher_.getTopic().empty()) {
				localization_diagnostics_msg_.header.frame_id = map_frame_id_;
				localization_diagnostics_msg_.header.stamp = ambient_cloud_time;
				localization_diagnostics_msg_.number_correspondences_last_registration_algorithm = number_correspondences_last_registration_algorithm_;
				localization_diagnostics_publisher_.publish(localization_diagnostics_msg_);
			}

			if (!aligned_pointcloud_publisher_.getTopic().empty()) {
				if (!publish_aligned_pointcloud_only_if_there_is_subscribers_ || (publish_aligned_pointcloud_only_if_there_is_subscribers_ && aligned_pointcloud_publisher_.getNumSubscribers() > 0)) {
					ROS_DEBUG_STREAM("Publishing registered ambient pointcloud with " << ambient_pointcloud->size() << " points");
					sensor_msgs::PointCloud2Ptr aligned_pointcloud_msg(new sensor_msgs::PointCloud2());
					pcl::toROSMsg(*ambient_pointcloud, *aligned_pointcloud_msg);
					aligned_pointcloud_msg->header.frame_id = map_frame_id_;
					aligned_pointcloud_publisher_.publish(aligned_pointcloud_msg);
				} else {
					ROS_DEBUG_STREAM("Avoiding publishing pointcloud on topic " << aligned_pointcloud_publisher_.getTopic() << " because there is no subscribers");
				}
			}

			performance_timer.restart();

			if (reference_pointcloud_required_ && !reference_pointcloud_loaded_ && map_update_mode_ != NoIntegration) {
				if (updateReferencePointCloudWithAmbientPointCloud(ambient_pointcloud, ambient_pointcloud_keypoints)) {
					reference_pointcloud_loaded_ = true;
				}
			} else {
				switch (map_update_mode_) {
					case FullIntegration: { updateReferencePointCloudWithAmbientPointCloud(ambient_pointcloud, ambient_pointcloud_keypoints); break; }
					case InliersIntegration: { if (registered_inliers_) { updateReferencePointCloudWithAmbientPointCloud(registered_inliers_, ambient_pointcloud_keypoints); } break; }
					case OutliersIntegration: { if (registered_outliers_) { updateReferencePointCloudWithAmbientPointCloud(registered_outliers_, ambient_pointcloud_keypoints); } break; }
					case NoIntegration: { break; }
				}
			}

			localization_times_msg_.map_update_time = performance_timer.getElapsedTimeInMilliSec();
		} else {
			if (ambient_pointcloud_with_circular_buffer_ && circular_buffer_clear_inserted_points_if_registration_fails_) {
				ambient_pointcloud_with_circular_buffer_->eraseNewest(last_number_points_inserted_in_circular_buffer_);
			}
			++pose_tracking_number_of_failed_registrations_since_last_valid_pose_;
			ROS_WARN_STREAM("Discarded cloud because localization couldn't be calculated");
		}

		received_external_initial_pose_estimation_ = false;
		accepted_pose_corrections_.clear();
	} catch (std::exception& e) {
		ROS_ERROR_STREAM("Exception caught in ambient pointcloud callback! Info: [" << e.what() <<"]");
		sensor_data_processing_status_ = ExceptionRaised;
	}

	if (sensor_data_processing_status_ == SuccessfulPreprocessing) {
		ROS_DEBUG("Successful finished point cloud preprocessing");
		return true;
	} else if (sensor_data_processing_status_ == SuccessfulPoseEstimation) {
		ROS_DEBUG("Successful finished pose estimation");
		return true;
	} else {
		ROS_WARN_STREAM("Failed point cloud processing with error [" << SensorDataProcessingStatusToStr(sensor_data_processing_status_) << "]");
		return false;
	}
}


template<typename PointT>
void Localization<PointT>::resetPointCloudHeight(pcl::PointCloud<PointT>& pointcloud, float height) {
	for (size_t i = 0; i < pointcloud.size(); ++i) {
		pointcloud.points[i].z = height;
	}
}


template<typename PointT>
bool Localization<PointT>::applyFilters(std::vector< typename CloudFilter<PointT>::Ptr >& cloud_filters, typename pcl::PointCloud<PointT>::Ptr& pointcloud) {
	ROS_DEBUG_STREAM("Filtering cloud in " << pointcloud->header.frame_id << " frame with " << pointcloud->size() << " points");

	PerformanceTimer performance_timer;
	performance_timer.start();

	for (size_t i = 0; i < cloud_filters.size(); ++i) {
		typename pcl::PointCloud<PointT>::Ptr filtered_ambient_pointcloud(new pcl::PointCloud<PointT>());
		filtered_ambient_pointcloud->header = pointcloud->header;
		cloud_filters[i]->filter(pointcloud, filtered_ambient_pointcloud);
		pointcloud = filtered_ambient_pointcloud; // switch pointers
		if (pointcloud->size() <= (size_t)minimum_number_of_points_in_ambient_pointcloud_)
			break;
	}

	localization_times_msg_.filtering_time += performance_timer.getElapsedTimeInMilliSec();

	return pointcloud->size() > (size_t)minimum_number_of_points_in_ambient_pointcloud_;
}


template<typename PointT>
bool Localization<PointT>::applyNormalEstimation(typename NormalEstimator<PointT>::Ptr& normal_estimator, typename CurvatureEstimator<PointT>::Ptr& curvature_estimator, typename pcl::PointCloud<PointT>::Ptr& pointcloud, typename pcl::PointCloud<PointT>::Ptr& surface,
		typename pcl::search::KdTree<PointT>::Ptr& pointcloud_search_method, bool pointcloud_is_map) {
	if (!normal_estimator && !curvature_estimator) return false;

	PerformanceTimer performance_timer;
	performance_timer.start();

	tf2::Transform sensor_pose_tf_guess;
	sensor_pose_tf_guess.setIdentity();
	ros::Time timestamp = pcl_conversions::fromPCL(pointcloud->header).stamp;
	if (!pointcloud_is_map) {
		if (pointcloud->header.frame_id != sensor_frame_id_) {
			if (pointcloud->header.frame_id == map_frame_id_ && pose_to_tf_publisher_->getTfCollector().lookForTransform(sensor_pose_tf_guess, odom_frame_id_, sensor_frame_id_, timestamp, tf_timeout_) && math_utils::isTransformValid(sensor_pose_tf_guess)) {
				sensor_pose_tf_guess = last_accepted_pose_odom_to_map_ * sensor_pose_tf_guess;
			} else if (pose_to_tf_publisher_->getTfCollector().lookForTransform(sensor_pose_tf_guess, pointcloud->header.frame_id, sensor_frame_id_, timestamp, tf_timeout_) && math_utils::isTransformValid(sensor_pose_tf_guess)) {
			} else if (pose_to_tf_publisher_->getTfCollector().lookForTransform(sensor_pose_tf_guess, odom_frame_id_, sensor_frame_id_, timestamp, tf_timeout_) && math_utils::isTransformValid(sensor_pose_tf_guess)) {
				sensor_pose_tf_guess = last_accepted_pose_odom_to_map_ * sensor_pose_tf_guess;
			} else {
				ROS_WARN_STREAM("Using identify for sensor pose when flipping normals to sensor viewpoint because TF [ " << sensor_frame_id_ << " -> " << pointcloud->header.frame_id << " is not available at timestamp " << timestamp);
				sensor_pose_tf_guess.setIdentity();
			}
		}
	}

	if (reference_pointcloud_2d_) {
		sensor_pose_tf_guess.getOrigin().setZ(0.0);
	}

	if (surface && surface->size() > (size_t)minimum_number_of_points_in_ambient_pointcloud_) {
		ROS_DEBUG_STREAM("Using raw pointcloud with " << surface->size() << " points as surface for normal estimation");
		typename pcl::search::KdTree<PointT>::Ptr surface_search_method(new pcl::search::KdTree<PointT>());
		surface_search_method->setInputCloud(surface);
		size_t number_surface_points = surface_search_method->getInputCloud()->size();
		if (normal_estimator) normal_estimator->estimateNormals(pointcloud, surface, surface_search_method, sensor_pose_tf_guess, pointcloud);
		if (curvature_estimator) curvature_estimator->estimatePointsCurvature(pointcloud, surface_search_method);

		if (number_surface_points != surface_search_method->getInputCloud()->size()) {
			pointcloud_search_method = surface_search_method; // normal estimator changed the number of pointcloud points and updated the search kd tree
		}
	} else {
		if (normal_estimator) normal_estimator->estimateNormals(pointcloud, pointcloud, pointcloud_search_method, sensor_pose_tf_guess, pointcloud);
		if (curvature_estimator) curvature_estimator->estimatePointsCurvature(pointcloud, pointcloud_search_method);
	}

	localization_times_msg_.surface_normal_estimation_time += performance_timer.getElapsedTimeInMilliSec();

	return pointcloud->size() > (size_t)minimum_number_of_points_in_ambient_pointcloud_;
}


template<typename PointT>
bool Localization<PointT>::applyKeypointDetection(std::vector< typename KeypointDetector<PointT>::Ptr >& keypoint_detectors, typename pcl::PointCloud<PointT>::Ptr& pointcloud, typename pcl::search::KdTree<PointT>::Ptr& surface_search_method, typename pcl::PointCloud<PointT>::Ptr& keypoints) {
	PerformanceTimer performance_timer;
	performance_timer.start();

	keypoints->clear();
	for (size_t i = 0; i < keypoint_detectors.size(); ++i) {
		if (i == 0) {
			keypoint_detectors[i]->findKeypoints(pointcloud, keypoints, pointcloud, surface_search_method);
		} else {
			typename pcl::PointCloud<PointT>::Ptr keypoints_temp(new pcl::PointCloud<PointT>());
			keypoint_detectors[i]->findKeypoints(pointcloud, keypoints_temp, pointcloud, surface_search_method);
			*keypoints += *keypoints_temp;
		}
	}

	localization_diagnostics_msg_.number_keypoints_ambient_pointcloud = keypoints->size();
	localization_times_msg_.keypoint_selection_time += performance_timer.getElapsedTimeInMilliSec();

	return keypoints->size() > 3;
}


template<typename PointT>
bool Localization<PointT>::applyCloudRegistration(std::vector< typename CloudMatcher<PointT>::Ptr >& matchers, typename pcl::PointCloud<PointT>::Ptr& ambient_pointcloud,
		typename pcl::search::KdTree<PointT>::Ptr& surface_search_method,
		typename pcl::PointCloud<PointT>::Ptr& pointcloud_keypoints,
		tf2::Transform& pose_corrections_in_out) {

	if (ambient_pointcloud->size() < (size_t)minimum_number_of_points_in_ambient_pointcloud_) { return false; }

	bool registration_successful = false;
	for (size_t i = 0; i < matchers.size(); ++i) {
		typename pcl::PointCloud<PointT>::Ptr ambient_pointcloud_aligned(new pcl::PointCloud<PointT>());
		tf2::Transform pose_correction;
		if (matchers[i]->registerCloud(ambient_pointcloud, surface_search_method, pointcloud_keypoints, pose_correction, accepted_pose_corrections_, ambient_pointcloud_aligned, false)) {
			pose_corrections_in_out = pose_correction * pose_corrections_in_out;
			registration_successful = true;
			ambient_pointcloud = ambient_pointcloud_aligned; // switch pointers
			surface_search_method->setInputCloud(ambient_pointcloud);
		} else {
			registration_successful = false;
		}

		int number_registration_iterations = matchers[i]->getNumberOfRegistrationIterations();
		if (number_registration_iterations > 0) number_of_registration_iterations_for_all_matchers_ += number_registration_iterations;

		double correspondence_estimation_time = matchers[i]->getCorrespondenceEstimationElapsedTimeMS();
		if (correspondence_estimation_time > 0) correspondence_estimation_time_for_all_matchers_ += correspondence_estimation_time;

		double transformation_estimation_time = matchers[i]->getTransformationEstimationElapsedTimeMS();
		if (transformation_estimation_time > 0) transformation_estimation_time_for_all_matchers_ += transformation_estimation_time;

		double transform_cloud_time = matchers[i]->getTransformCloudElapsedTimeMS();
		if (transform_cloud_time > 0) transform_cloud_time_for_all_matchers_ += transform_cloud_time;

		double cloud_align_time = matchers[i]->getCloudAlignTimeMS();
		if (cloud_align_time > 0) cloud_align_time_for_all_matchers_ += cloud_align_time;


		last_matcher_convergence_state_ = matchers[i]->getMatcherConvergenceState();
		root_mean_square_error_of_last_registration_correspondences_ = matchers[i]->getRootMeanSquareErrorOfRegistrationCorrespondences();
		number_correspondences_last_registration_algorithm_ = matchers[i]->getNumberCorrespondencesInLastRegistrationIteration();
	}

	return registration_successful;
}


template<typename PointT>
bool Localization<PointT>::postProcessCloudRegistration(const tf2::Transform& pointcloud_pose_initial_guess, const tf2::Transform& pointcloud_pose_corrected, tf2::Transform& new_pose_corrections_out, const ros::Time& pointcloud_time) {
	tf2::Transform new_transform = pointcloud_pose_corrected;
	new_pose_corrections_out.setIdentity();

	if (ignore_height_corrections_) {
		new_pose_corrections_out.getOrigin().setZ(pointcloud_pose_initial_guess.getOrigin().getZ() - new_transform.getOrigin().getZ());
		new_transform.getOrigin().setZ(pointcloud_pose_initial_guess.getOrigin().getZ());
	}
	new_transform.getRotation().normalize();

	if (transformation_aligner_) {
		if (!transformation_aligner_->alignTransformation(new_transform, pointcloud_time, base_link_frame_id_, map_frame_id_, pose_to_tf_publisher_->getTfCollector(), tf_timeout_)) {
			sensor_data_processing_status_ = FailedTransformationAligner;
			return false;
		}

		Eigen::Transform<double, 3, Eigen::Affine> new_transform_eigen = laserscan_to_pointcloud::tf_rosmsg_eigen_conversions::transformTF2ToTransform<double>(new_transform);
		Eigen::Transform<double, 3, Eigen::Affine> pointcloud_pose_corrected_eigen = laserscan_to_pointcloud::tf_rosmsg_eigen_conversions::transformTF2ToTransform<double>(pointcloud_pose_corrected);
		Eigen::Transform<double, 3, Eigen::Affine> new_pose_corrections_eigen = new_transform_eigen * pointcloud_pose_corrected_eigen.inverse();
		new_pose_corrections_out.setFromOpenGLMatrix(new_pose_corrections_eigen.matrix().data());
	}

	Eigen::Transform<double, 3, Eigen::Affine> new_pose_corrections_out_eigen = laserscan_to_pointcloud::tf_rosmsg_eigen_conversions::transformTF2ToTransform<double>(new_pose_corrections_out);
	std::string transform_string = math_utils::convertTransformToString<double>(new_pose_corrections_out_eigen.matrix());
	ROS_DEBUG_STREAM("Pointcloud registration postprocessing applied correction matrix:" << transform_string << "\n");

	return true;
}


template<typename PointT>
double Localization<PointT>::applyOutlierDetection(std::vector< typename OutlierDetector<PointT>::Ptr >& detectors,
		typename pcl::search::KdTree<PointT>::Ptr& reference_pointcloud_search_method, typename pcl::PointCloud<PointT>::Ptr& pointcloud,
		std::vector< typename pcl::PointCloud<PointT>::Ptr >& detected_outliers, std::vector< typename pcl::PointCloud<PointT>::Ptr >& detected_inliers,
		double& root_mean_square_error_inliers, size_t& number_inliers) {
	detected_outliers.clear();
	detected_inliers.clear();
	root_mean_square_error_inliers = std::numeric_limits<double>::max();
	number_inliers = 0;

	size_t number_outliers = 0;
	for (size_t i = 0; i < detectors.size(); ++i) {
		typename pcl::PointCloud<PointT>::Ptr outliers;
		typename pcl::PointCloud<PointT>::Ptr inliers;

		if (detectors[i]->isPublishingOutliers() || compute_outliers_angular_distribution_) {
			outliers = typename pcl::PointCloud<PointT>::Ptr(new pcl::PointCloud<PointT>());
			outliers->header = pointcloud->header;
			outliers->header.frame_id = map_frame_id_;
		}

		if (detectors[i]->isPublishingInliers() || compute_inliers_angular_distribution_) {
			inliers = typename pcl::PointCloud<PointT>::Ptr(new pcl::PointCloud<PointT>());
			inliers->header = pointcloud->header;
			inliers->header.frame_id = map_frame_id_;
		}

		number_outliers += detectors[i]->detectOutliers(reference_pointcloud_search_method, *pointcloud, outliers, inliers, root_mean_square_error_inliers);
		detected_outliers.push_back(outliers);
		detected_inliers.push_back(inliers);
	}

	number_inliers = pointcloud->size() - number_outliers;
	double outlier_ratio = (double)number_outliers / (double) (pointcloud->size());
	if (outlier_ratio < 0.0 || outlier_ratio > 1.0) { outlier_ratio = 1.0; }
	return outlier_ratio;
}


template<typename PointT>
void Localization<PointT>::applyAmbientPointCloudOutlierDetection(typename pcl::PointCloud<PointT>::Ptr& ambient_pointcloud) {
	if (!outlier_detectors_.empty()) {
		if (!reference_pointcloud_search_method_ || (reference_pointcloud_search_method_ && !(reference_pointcloud_search_method_->getInputCloud()))) {
			ROS_WARN("Missing reference point cloud for computing ambient point cloud outliers");
			return;
		}

		outlier_percentage_ = applyOutlierDetection(outlier_detectors_, reference_pointcloud_search_method_, ambient_pointcloud, detected_outliers_, detected_inliers_, root_mean_square_error_inliers_, number_inliers_);
		if (detected_inliers_.size() > 1) {
			registered_inliers_ = typename pcl::PointCloud<PointT>::Ptr(new pcl::PointCloud<PointT>());
			pointcloud_utils::concatenatePointClouds<PointT>(detected_inliers_, registered_inliers_);
		} else if (detected_inliers_.size() == 1) {
			registered_inliers_ = detected_inliers_[0];
		} else {
			if (registered_inliers_) registered_inliers_->clear();
		}
	}
}


template<typename PointT>
void Localization<PointT>::applyReferencePointCloudOutlierDetection(typename pcl::search::KdTree<PointT>::Ptr& ambient_pointcloud_search_method, typename pcl::PointCloud<PointT>::Ptr& reference_pointcloud) {
	if (!outlier_detectors_reference_pointcloud_.empty()) {
		if (!reference_pointcloud_) {
			ROS_WARN("Missing reference point cloud for computing reference point cloud outliers");
			return;
		}

		outlier_percentage_reference_pointcloud_ = applyOutlierDetection(outlier_detectors_reference_pointcloud_, ambient_pointcloud_search_method, reference_pointcloud, detected_outliers_reference_pointcloud_, detected_inliers_reference_pointcloud_, root_mean_square_error_inliers_reference_pointcloud_, number_inliers_reference_pointcloud_);
	}
}


template<typename PointT>
bool Localization<PointT>::applyCloudAnalysis(const tf2::Transform& estimated_pose) {
	bool performed_analysis = false;
	inliers_angular_distribution_ = 2.0;
	outliers_angular_distribution_ = -2.0;

	if (cloud_analyzer_) {
		if (compute_outliers_angular_distribution_ && registered_outliers_ && !detected_outliers_.empty()) {
			std::vector<size_t> analysis_histogram;
			outliers_angular_distribution_ = cloud_analyzer_->analyzeCloud(estimated_pose, *registered_outliers_, analysis_histogram);
			performed_analysis = true;
		}

		if (compute_inliers_angular_distribution_ && registered_inliers_ && !detected_inliers_.empty()) {
			std::vector<size_t> analysis_histogram;
			inliers_angular_distribution_ = cloud_analyzer_->analyzeCloud(estimated_pose, *registered_inliers_, analysis_histogram);
			performed_analysis = true;
		}
	}

	return performed_analysis;
}


template<typename PointT>
void Localization<PointT>::publishDetectedOutliers() {
	if (outlier_detectors_.size() == detected_outliers_.size()) {
		for (size_t i = 0; i < detected_outliers_.size(); ++i) {
			outlier_detectors_[i]->publishOutliers(detected_outliers_[i]);
		}
	}

	if (outlier_detectors_reference_pointcloud_.size() == detected_outliers_reference_pointcloud_.size()) {
		for (size_t i = 0; i < detected_outliers_reference_pointcloud_.size(); ++i) {
			outlier_detectors_reference_pointcloud_[i]->publishOutliers(detected_outliers_reference_pointcloud_[i]);
		}
	}

	detected_outliers_.clear();
	detected_outliers_reference_pointcloud_.clear();
}


template<typename PointT>
void Localization<PointT>::publishDetectedInliers() {
	if (outlier_detectors_.size() == detected_inliers_.size()) {
		for (size_t i = 0; i < detected_inliers_.size(); ++i) {
			outlier_detectors_[i]->publishInliers(detected_inliers_[i]);
		}
	}

	if (outlier_detectors_reference_pointcloud_.size() == detected_inliers_reference_pointcloud_.size()) {
		for (size_t i = 0; i < detected_inliers_reference_pointcloud_.size(); ++i) {
			outlier_detectors_reference_pointcloud_[i]->publishInliers(detected_inliers_reference_pointcloud_[i]);
		}
	}

	detected_inliers_.clear();
	detected_inliers_reference_pointcloud_.clear();
}


template<typename PointT>
bool Localization<PointT>::applyTransformationValidators(std::vector< TransformationValidator::Ptr >& transformation_validators, const tf2::Transform& pointcloud_pose_initial_guess, tf2::Transform& pointcloud_pose_corrected_in_out, double max_outlier_percentage, double max_outlier_percentage_reference_pointcloud) {
	for (size_t i = 0; i < transformation_validators.size(); ++i) {
		if (last_accepted_pose_valid_ && (ros::Time::now() - last_accepted_pose_time_ < pose_tracking_timeout_)) {
			if (!transformation_validators[i]->validateNewLocalizationPose(last_accepted_pose_base_link_to_map_, pointcloud_pose_initial_guess, pointcloud_pose_corrected_in_out, root_mean_square_error_inliers_, root_mean_square_error_inliers_reference_pointcloud_, max_outlier_percentage, max_outlier_percentage_reference_pointcloud, inliers_angular_distribution_, outliers_angular_distribution_)) {
				return false;
			}
		} else {
			// lost tracking -> ignore last pose filtering -> use only rmse and outlier percentage
			if (!transformation_validators[i]->validateNewLocalizationPose(pointcloud_pose_corrected_in_out, pointcloud_pose_corrected_in_out, pointcloud_pose_corrected_in_out, root_mean_square_error_inliers_, root_mean_square_error_inliers_reference_pointcloud_, max_outlier_percentage, max_outlier_percentage_reference_pointcloud, inliers_angular_distribution_, outliers_angular_distribution_)) {
				return false;
			}
		}
	}

	return true;
}


template<typename PointT>
void Localization<PointT>::fillPoseCovariance(geometry_msgs::PoseWithCovarianceStamped& pose_corrected_msg, Eigen::MatrixXd& covariance_matrix) {
	/*double covariance = root_mean_square_error_inliers_ * 2.0;
	pose_corrected_msg.pose.covariance[0] = covariance;
	pose_corrected_msg.pose.covariance[7] = covariance;
	pose_corrected_msg.pose.covariance[14] = covariance;
	pose_corrected_msg.pose.covariance[21] = covariance;
	pose_corrected_msg.pose.covariance[28] = covariance;
	pose_corrected_msg.pose.covariance[35] = covariance;*/

	Eigen::MatrixXd covariance_matrix_row_major = covariance_matrix.transpose(); // eigen uses column-major storage while ros uses row-major storage
	size_t max_i = std::min((size_t)pose_corrected_msg.pose.covariance.size(), (size_t)(covariance_matrix_row_major.cols() * covariance_matrix_row_major.rows()));
	for (size_t i = 0; i < max_i; ++i) {
		pose_corrected_msg.pose.covariance[i] = covariance_matrix_row_major.data()[i];
	}
}


template<typename PointT>
bool Localization<PointT>::updateLocalizationWithAmbientPointCloud(typename pcl::PointCloud<PointT>::Ptr& ambient_pointcloud, const ros::Time& pointcloud_time, const tf2::Transform& pointcloud_pose_initial_guess,
		tf2::Transform& pointcloud_pose_corrected_out, tf2::Transform& pose_corrections_out, typename pcl::PointCloud<PointT>::Ptr ambient_pointcloud_keypoints_out) {
	last_number_points_inserted_in_circular_buffer_ = 0;
	localization_diagnostics_msg_.number_keypoints_ambient_pointcloud = 0;
	localization_diagnostics_msg_.number_points_ambient_pointcloud = ambient_pointcloud->size();
	pointcloud_pose_corrected_out = pointcloud_pose_initial_guess;
	accepted_pose_corrections_.clear();
	pose_corrections_out = tf2::Transform::getIdentity();
	std::string ambient_point_cloud_original_frame_id = ambient_pointcloud->header.frame_id;
	reference_pointcloud_->header.frame_id = map_frame_id_;
	reference_pointcloud_->header.stamp = pcl_conversions::toPCL(pointcloud_time);

	if (ambient_pointcloud->size() < (size_t)minimum_number_of_points_in_ambient_pointcloud_) {
		sensor_data_processing_status_ = PointCloudWithoutTheMinimumNumberOfRequiredPoints;
		return false;
	}

	last_accepted_pose_performed_tracking_reset_ = false;
	bool lost_tracking = checkIfTrackingIsLost();

	typename pcl::PointCloud<PointT>::Ptr ambient_pointcloud_raw;
	if (ambient_cloud_normal_estimator_ && (compute_normals_when_tracking_pose_ || compute_normals_when_estimating_initial_pose_ || compute_normals_when_recovering_pose_tracking_)) {
		if (use_filtered_cloud_as_normal_estimation_surface_ambient_) {
			ROS_DEBUG("Using filtered ambient point cloud for normal estimation");
		} else {
			ROS_DEBUG("Using raw ambient point cloud for normal estimation");
			ambient_pointcloud_raw = typename pcl::PointCloud<PointT>::Ptr(new pcl::PointCloud<PointT>(*ambient_pointcloud));
			if (!transformCloudToTFFrame(ambient_pointcloud_raw, pointcloud_time, map_frame_id_for_transforming_pointclouds_)) {
				sensor_data_processing_status_ = FailedTFTransform;
				return false;
			}
			if (reference_pointcloud_2d_) { resetPointCloudHeight(*ambient_pointcloud_raw); }
		}
	}

	// ==============================================================  filters integration
	typename pcl::PointCloud<PointT>::Ptr ambient_pointcloud_integration;
	if (!ambient_pointcloud_integration_filters_.empty() || !ambient_pointcloud_integration_filters_map_frame_.empty()) {
		ROS_DEBUG("Using a pointcloud with different filters for SLAM");

		if (ambient_pointcloud_integration_filters_preprocessed_pointcloud_save_original_pointcloud_ && !ambient_pointcloud_integration_filters_preprocessed_pointcloud_save_filename_.empty()) {
			ROS_DEBUG_STREAM("Saving original point cloud with " << ambient_pointcloud->size() << " points to " << reference_pointclouds_database_folder_path_ + ambient_pointcloud_integration_filters_preprocessed_pointcloud_save_filename_ + "_raw");
			pointcloud_conversions::toFile(ambient_pointcloud_integration_filters_preprocessed_pointcloud_save_filename_ + "_raw", *ambient_pointcloud, save_reference_pointclouds_in_binary_format_, reference_pointclouds_database_folder_path_);
		}

		ambient_pointcloud_integration = typename pcl::PointCloud<PointT>::Ptr(new pcl::PointCloud<PointT>(*ambient_pointcloud));
		if (!applyFilters(ambient_pointcloud_integration_filters_, ambient_pointcloud_integration)) {
			sensor_data_processing_status_ = PointCloudFilteringFailed;
			return false;
		}
		if (!ambient_pointcloud_filters_custom_frame_id_.empty()) {
			if (!transformCloudToTFFrame(ambient_pointcloud_integration, pointcloud_time, ambient_pointcloud_filters_custom_frame_id_)) {
				sensor_data_processing_status_ = FailedTFTransform;
				return false;
			}
			if (!applyFilters(ambient_pointcloud_filters_custom_frame_, ambient_pointcloud_integration)) {
				sensor_data_processing_status_ = PointCloudFilteringFailed;
				return false;
			}
		}
		if (!transformCloudToTFFrame(ambient_pointcloud_integration, pointcloud_time, map_frame_id_for_transforming_pointclouds_)) {
			sensor_data_processing_status_ = FailedTFTransform;
			return false;
		}
		if (!applyFilters(ambient_pointcloud_integration_filters_map_frame_, ambient_pointcloud_integration)) {
			sensor_data_processing_status_ = PointCloudFilteringFailed;
			return false;
		}
		if (reference_pointcloud_2d_) { resetPointCloudHeight(*ambient_pointcloud_integration); }
	}

	// ==============================================================  normal estimation integration
	if (ambient_pointcloud_integration) {
		typename pcl::search::KdTree<PointT>::Ptr ambient_integration_search_method(new pcl::search::KdTree<PointT>());
		ambient_integration_search_method->setInputCloud(ambient_pointcloud_integration);

		if (ambient_cloud_normal_estimator_ || ambient_cloud_curvature_estimator_) {
			if (!applyNormalEstimation(ambient_cloud_normal_estimator_, ambient_cloud_curvature_estimator_, ambient_pointcloud_integration, ambient_pointcloud_raw, ambient_integration_search_method)) {
				sensor_data_processing_status_ = FailedNormalEstimation;
				return false;
			}
		}

		if (!applyFilters(ambient_pointcloud_filters_after_normal_estimation_, ambient_pointcloud_integration)) {
			sensor_data_processing_status_ = PointCloudFilteringFailed;
			return false;
		}

		std::vector<int> indexes;
		pcl::removeNaNFromPointCloud(*ambient_pointcloud_integration, *ambient_pointcloud_integration, indexes);
		indexes.clear();
		pcl::removeNaNNormalsFromPointCloud(*ambient_pointcloud_integration, *ambient_pointcloud_integration, indexes);
		indexes.clear();

		if (!ambient_pointcloud_integration_filters_preprocessed_pointcloud_save_filename_.empty()) {
			ROS_DEBUG_STREAM("Saving preprocessed point cloud with " << ambient_pointcloud_integration->size() << " points to " << reference_pointclouds_database_folder_path_ + ambient_pointcloud_integration_filters_preprocessed_pointcloud_save_filename_);
			pointcloud_conversions::toFile(ambient_pointcloud_integration_filters_preprocessed_pointcloud_save_filename_, *ambient_pointcloud_integration, save_reference_pointclouds_in_binary_format_, reference_pointclouds_database_folder_path_);
		}
	}

	// ==============================================================  FirstPointCloudInSlamMode
	if ((reference_pointcloud_required_ && (!reference_pointcloud_loaded_ || reference_pointcloud_->size() < (size_t)minimum_number_of_points_in_reference_pointcloud_)) ||
			!cloudMatchersActive()) {
		if (ambient_pointcloud_integration) {
			ROS_DEBUG("Switching SLAM cloud");
			ambient_pointcloud = ambient_pointcloud_integration;
		}

		last_accepted_pose_base_link_to_map_ = pointcloud_pose_corrected_out;
		last_accepted_pose_time_ = ros::Time::now();
		last_accepted_pose_valid_ = true;
		robot_initial_pose_available_ = true;
		received_external_initial_pose_estimation_ = false;
		pose_tracking_number_of_failed_registrations_since_last_valid_pose_ = 0;

		if (cloudMatchersActive()) {
			sensor_data_processing_status_ = FirstPointCloudInSlamMode;
			return false;
		} else {
			sensor_data_processing_status_ = SuccessfulPreprocessing;
			return true;
		}
		// stop pipeline processing if no reference cloud is available (only before registration to allow preprocessing of the first cloud when performing SLAM)
	}

	// ==============================================================  filters
	if (!applyFilters(lost_tracking ? ambient_pointcloud_feature_registration_filters_ : ambient_pointcloud_filters_, ambient_pointcloud)) {
		sensor_data_processing_status_ = PointCloudFilteringFailed;
		return false;
	}
	if (!ambient_pointcloud_filters_custom_frame_id_.empty()) {
		if (!transformCloudToTFFrame(ambient_pointcloud, pointcloud_time, ambient_pointcloud_filters_custom_frame_id_)) {
			sensor_data_processing_status_ = FailedTFTransform;
			return false;
		}
		if (!applyFilters(ambient_pointcloud_filters_custom_frame_, ambient_pointcloud)) {
			sensor_data_processing_status_ = PointCloudFilteringFailed;
			return false;
		}
	}
	if (!transformCloudToTFFrame(ambient_pointcloud, pointcloud_time, map_frame_id_for_transforming_pointclouds_)) {
		sensor_data_processing_status_ = FailedTFTransform;
		return false;
	}
	if (!applyFilters(lost_tracking ? ambient_pointcloud_map_frame_feature_registration_filters_ : ambient_pointcloud_filters_map_frame_, ambient_pointcloud)) {
		sensor_data_processing_status_ = PointCloudFilteringFailed;
		return false;
	}
	if (reference_pointcloud_2d_) { resetPointCloudHeight(*ambient_pointcloud); }

	localization_diagnostics_msg_.number_points_ambient_pointcloud_after_filtering = ambient_pointcloud->size();
	if (ambient_pointcloud_with_circular_buffer_) {
		ambient_pointcloud_with_circular_buffer_->insert(*ambient_pointcloud);
		ambient_pointcloud_with_circular_buffer_->getPointCloud().header = ambient_pointcloud->header;
		ambient_pointcloud_with_circular_buffer_->getPointCloud().header.frame_id = map_frame_id_;
		ambient_pointcloud_with_circular_buffer_->getPointCloud().sensor_origin_ = ambient_pointcloud->sensor_origin_;
		ambient_pointcloud_with_circular_buffer_->getPointCloud().sensor_orientation_ = ambient_pointcloud->sensor_orientation_;
		last_number_points_inserted_in_circular_buffer_ = ambient_pointcloud->size();
		ambient_pointcloud = ambient_pointcloud_with_circular_buffer_->getPointCloudPtr();
		ROS_DEBUG_STREAM("Ambient pointcloud with circular buffer has " << ambient_pointcloud->size() << " points");
	}

	// ==============================================================  check point cloud size
	if (ambient_pointcloud_with_circular_buffer_ && circular_buffer_require_reception_of_pointcloud_msgs_from_all_topics_before_doing_registration_) {
		msg_frame_ids_with_data_in_circular_buffer_.insert(ambient_point_cloud_original_frame_id);
		if (msg_frame_ids_with_data_in_circular_buffer_.size() < ambient_pointcloud_subscribers_.size()) {
			ROS_DEBUG_STREAM("Added frame_id " << ambient_point_cloud_original_frame_id << " to the set containing the received frame_ids with data in the circular buffer");
			sensor_data_processing_status_ = FillingCircularBufferWithMsgsFromAllTopics;
			return false;
		} else {
			ROS_DEBUG("Received msgs in all the subscribed point cloud topics");
			msg_frame_ids_with_data_in_circular_buffer_.clear();
		}
	}
	if (ambient_pointcloud->size() < (size_t)minimum_number_of_points_in_ambient_pointcloud_ || ambient_pointcloud->size() < (size_t)minimum_number_points_ambient_pointcloud_circular_buffer_) {
		sensor_data_processing_status_ = PointCloudWithoutTheMinimumNumberOfRequiredPoints;
		return false;
	}

	// ==============================================================  normal estimation
	typename pcl::search::KdTree<PointT>::Ptr ambient_search_method(new pcl::search::KdTree<PointT>());
	ambient_search_method->setInputCloud(ambient_pointcloud);
	bool computed_normals = false;
	localization_times_msg_.surface_normal_estimation_time = 0.0;
	if (compute_normals_when_tracking_pose_ && (ambient_cloud_normal_estimator_ || ambient_cloud_curvature_estimator_)) {
		if (!applyNormalEstimation(ambient_cloud_normal_estimator_, ambient_cloud_curvature_estimator_, ambient_pointcloud, ambient_pointcloud_raw, ambient_search_method)) {
			sensor_data_processing_status_ = FailedNormalEstimation;
			return false;
		}
		computed_normals = true;
	}

	if (!applyFilters(ambient_pointcloud_filters_after_normal_estimation_, ambient_pointcloud)) {
		sensor_data_processing_status_ = PointCloudFilteringFailed;
		return false;
	}

	std::vector<int> indexes;
	pcl::removeNaNFromPointCloud(*ambient_pointcloud, *ambient_pointcloud, indexes);
	indexes.clear();
	pcl::removeNaNNormalsFromPointCloud(*ambient_pointcloud, *ambient_pointcloud, indexes);
	indexes.clear();

	if (!filtered_pointcloud_publisher_.getTopic().empty()) {
		if (!publish_filtered_pointcloud_only_if_there_is_subscribers_ || (publish_filtered_pointcloud_only_if_there_is_subscribers_ && filtered_pointcloud_publisher_.getNumSubscribers() > 0)) {
			ROS_DEBUG_STREAM("Publishing filtered ambient pointcloud with " << ambient_pointcloud->size() << " points");
			sensor_msgs::PointCloud2Ptr filtered_pointcloud_msg(new sensor_msgs::PointCloud2());
			pcl::toROSMsg(*ambient_pointcloud, *filtered_pointcloud_msg);
			filtered_pointcloud_msg->header.frame_id = map_frame_id_for_publishing_pointclouds_;
			filtered_pointcloud_publisher_.publish(filtered_pointcloud_msg);
		} else {
			ROS_DEBUG_STREAM("Avoiding publishing pointcloud on topic " << filtered_pointcloud_publisher_.getTopic() << " because there is no subscribers");
		}
	}

	// ==============================================================  keypoint selection
	localization_times_msg_.keypoint_selection_time = 0.0;
	bool computed_keypoints = false;
	if (compute_keypoints_when_tracking_pose_ && !ambient_cloud_keypoint_detectors_.empty()) {
		applyKeypointDetection(ambient_cloud_keypoint_detectors_, ambient_pointcloud, ambient_search_method, ambient_pointcloud_keypoints_out);
		computed_keypoints = true;
	}

	// ==============================================================  initial pose estimation when tracking is lost
	localization_diagnostics_msg_.number_points_ambient_pointcloud_used_in_registration = ambient_pointcloud->size();
	PerformanceTimer performance_timer;
	performance_timer.start();

	bool tracking_recovery_reached = ((ros::Time::now() - last_accepted_pose_time_) > pose_tracking_recovery_timeout_ && pose_tracking_number_of_failed_registrations_since_last_valid_pose_ > pose_tracking_recovery_minimum_number_of_failed_registrations_since_last_valid_pose_) || (pose_tracking_number_of_failed_registrations_since_last_valid_pose_ > pose_tracking_recovery_maximum_number_of_failed_registrations_since_last_valid_pose_);
	bool performed_recovery = false;
	number_of_registration_iterations_for_all_matchers_ = 0;
	correspondence_estimation_time_for_all_matchers_ = 0;
	transformation_estimation_time_for_all_matchers_ = 0;
	transform_cloud_time_for_all_matchers_ = 0;
	cloud_align_time_for_all_matchers_ = 0;
	last_matcher_convergence_state_ = "";
	root_mean_square_error_of_last_registration_correspondences_ = -1.0;
	number_correspondences_last_registration_algorithm_ = -1;
	ambient_pointcloud->header.frame_id = map_frame_id_;

	if ((!initial_pose_estimators_feature_matchers_.empty() || !initial_pose_estimators_point_matchers_.empty()) && (lost_tracking || received_external_initial_pose_estimation_)) { // lost tracking -> try to find initial pose
		if (!received_external_initial_pose_estimation_ && !initial_pose_estimators_feature_matchers_.empty()) {
			ros::Duration time_from_last_pose = ros::Time::now() - last_accepted_pose_time_;
			if (time_from_last_pose < initial_pose_estimation_timeout_) {
				ROS_INFO("Performing initial pose recovery");
				ambient_pointcloud->header.frame_id = map_frame_id_for_publishing_pointclouds_;
				if (!computed_normals && compute_normals_when_estimating_initial_pose_ && (ambient_cloud_normal_estimator_ || ambient_cloud_curvature_estimator_)) {
					if (!applyNormalEstimation(ambient_cloud_normal_estimator_, ambient_cloud_curvature_estimator_, ambient_pointcloud, ambient_pointcloud_raw, ambient_search_method)) {
						sensor_data_processing_status_ = FailedNormalEstimation;
						return false;
					}
					computed_normals = true;
				}

				if (!computed_keypoints && compute_keypoints_when_estimating_initial_pose_ && !ambient_cloud_keypoint_detectors_.empty()) {
					applyKeypointDetection(ambient_cloud_keypoint_detectors_, ambient_pointcloud, ambient_search_method, ambient_pointcloud_keypoints_out);
					computed_keypoints = true;
				}

				ambient_pointcloud->header.frame_id = map_frame_id_;
				applyCloudRegistration(initial_pose_estimators_feature_matchers_, ambient_pointcloud, ambient_search_method, (ambient_pointcloud_keypoints_out->size() < (size_t)minimum_number_of_points_in_ambient_pointcloud_) ? ambient_pointcloud : ambient_pointcloud_keypoints_out, pose_corrections_out);
			} else {
				ROS_DEBUG_STREAM("Timeout of " << initial_pose_estimation_timeout_ << " seconds reached (" << time_from_last_pose << " seconds from last valid pose)");
			}
		}

		if (!initial_pose_estimators_point_matchers_.empty() && !applyCloudRegistration(initial_pose_estimators_point_matchers_, ambient_pointcloud, ambient_search_method, (ambient_pointcloud_keypoints_out->size() < (size_t)minimum_number_of_points_in_ambient_pointcloud_) ? ambient_pointcloud : ambient_pointcloud_keypoints_out, pose_corrections_out)) {
			sensor_data_processing_status_ = FailedInitialPoseEstimation;
			return false;
		}

		localization_times_msg_.initial_pose_estimation_time = performance_timer.getElapsedTimeInMilliSec();
		last_accepted_pose_performed_tracking_reset_ = true;
		ROS_DEBUG("Successfully performed initial pose estimation");
	} else {
		// ==============================================================  point cloud registration with recovery
		performance_timer.restart();
		localization_times_msg_.pointcloud_registration_time = 0.0;

		if ((!tracking_matchers_.empty() || !tracking_recovery_matchers_.empty()) && !applyCloudRegistration(tracking_matchers_, ambient_pointcloud, ambient_search_method, (ambient_pointcloud_keypoints_out->size() < (size_t)minimum_number_of_points_in_ambient_pointcloud_) ? ambient_pointcloud : ambient_pointcloud_keypoints_out, pose_corrections_out)) {
			if (tracking_recovery_matchers_.empty()) {
				sensor_data_processing_status_ = FailedPoseEstimation;
				return false;
			} else if (tracking_recovery_reached) {
				ambient_pointcloud->header.frame_id = map_frame_id_for_publishing_pointclouds_;
				localization_times_msg_.pointcloud_registration_time += performance_timer.getElapsedTimeInMilliSec();
				if (!computed_normals && compute_normals_when_recovering_pose_tracking_ && (ambient_cloud_normal_estimator_ || ambient_cloud_curvature_estimator_)) {
					if (!applyNormalEstimation(ambient_cloud_normal_estimator_, ambient_cloud_curvature_estimator_, ambient_pointcloud, ambient_pointcloud_raw, ambient_search_method)) {
						sensor_data_processing_status_ = FailedNormalEstimation;
						return false;
					}
					computed_normals = true;
				}

				if (!computed_keypoints && compute_keypoints_when_recovering_pose_tracking_ && !ambient_cloud_keypoint_detectors_.empty()) {
					applyKeypointDetection(ambient_cloud_keypoint_detectors_, ambient_pointcloud, ambient_search_method, ambient_pointcloud_keypoints_out);
					computed_keypoints = true;
				}

				performance_timer.restart();
				ambient_pointcloud->header.frame_id = map_frame_id_;
				if (applyCloudRegistration(tracking_recovery_matchers_, ambient_pointcloud, ambient_search_method, (ambient_pointcloud_keypoints_out->size() < (size_t)minimum_number_of_points_in_ambient_pointcloud_) ? ambient_pointcloud : ambient_pointcloud_keypoints_out, pose_corrections_out)) {
					ROS_INFO("Successfully performed registration recovery");
					performed_recovery = true;
					localization_times_msg_.pointcloud_registration_time += performance_timer.getElapsedTimeInMilliSec();
				} else {
					sensor_data_processing_status_ = FailedPoseEstimation;
					return false;
				}
			}
		} else {
			localization_times_msg_.pointcloud_registration_time += performance_timer.getElapsedTimeInMilliSec();
		}
	}

	pointcloud_pose_corrected_out = pose_corrections_out * pointcloud_pose_initial_guess;
	tf2::Transform post_process_cloud_registration_pose_corrections;
	if (!postProcessCloudRegistration(pointcloud_pose_initial_guess, pointcloud_pose_corrected_out, post_process_cloud_registration_pose_corrections, pointcloud_time)) { return false; }
	pcl::transformPointCloudWithNormals(*ambient_pointcloud, *ambient_pointcloud, laserscan_to_pointcloud::tf_rosmsg_eigen_conversions::transformTF2ToTransform<double>(post_process_cloud_registration_pose_corrections));
	pcl::transformPointCloudWithNormals(*ambient_pointcloud_keypoints_out, *ambient_pointcloud_keypoints_out, laserscan_to_pointcloud::tf_rosmsg_eigen_conversions::transformTF2ToTransform<double>(post_process_cloud_registration_pose_corrections));
	pose_corrections_out = post_process_cloud_registration_pose_corrections * pose_corrections_out;
	pointcloud_pose_corrected_out = pose_corrections_out * pointcloud_pose_initial_guess;

	if (ambient_pointcloud_integration) {
		pcl::transformPointCloudWithNormals(*ambient_pointcloud_integration, *ambient_pointcloud_integration, laserscan_to_pointcloud::tf_rosmsg_eigen_conversions::transformTF2ToTransform<double>(pose_corrections_out));
	}

	// ==============================================================  outlier detection
	performance_timer.restart();
	applyAmbientPointCloudOutlierDetection(ambient_pointcloud_integration ? ambient_pointcloud_integration : ambient_pointcloud);
	applyReferencePointCloudOutlierDetection(ambient_search_method, reference_pointcloud_);
	localization_times_msg_.outlier_detection_time = performance_timer.getElapsedTimeInMilliSec();


	// ==============================================================  localization post processors with registration recovery
	performance_timer.restart();
	applyCloudAnalysis(pointcloud_pose_corrected_out);
	localization_times_msg_.registered_points_angular_distribution_analysis_time = performance_timer.getElapsedTimeInMilliSec();

	performance_timer.restart();
	localization_times_msg_.transformation_validators_time = 0.0;
	if (performed_recovery && !transformation_validators_tracking_recovery_.empty()) {
		if (!applyTransformationValidators(transformation_validators_tracking_recovery_, pointcloud_pose_initial_guess, pointcloud_pose_corrected_out, outlier_percentage_, outlier_percentage_reference_pointcloud_)) {
			sensor_data_processing_status_ = PoseEstimationRejectedByTransformationValidators;
			return false;
		}
	} else {
		if (!applyTransformationValidators(lost_tracking ? transformation_validators_initial_alignment_ : transformation_validators_, pointcloud_pose_initial_guess, pointcloud_pose_corrected_out, outlier_percentage_, outlier_percentage_reference_pointcloud_)) {
			localization_times_msg_.transformation_validators_time = performance_timer.getElapsedTimeInMilliSec();
			performance_timer.restart();
			if (!performed_recovery && !tracking_recovery_matchers_.empty() && tracking_recovery_reached) {
				ambient_pointcloud->header.frame_id = map_frame_id_for_publishing_pointclouds_;
				if (!computed_normals && compute_normals_when_recovering_pose_tracking_ && (ambient_cloud_normal_estimator_ || ambient_cloud_curvature_estimator_)) {
					if (!applyNormalEstimation(ambient_cloud_normal_estimator_, ambient_cloud_curvature_estimator_, ambient_pointcloud, ambient_pointcloud_raw, ambient_search_method)) {
						sensor_data_processing_status_ = FailedNormalEstimation;
						return false;
					}
					computed_normals = true;
				}

				if (!computed_keypoints && compute_keypoints_when_recovering_pose_tracking_ && !ambient_cloud_keypoint_detectors_.empty()) {
					applyKeypointDetection(ambient_cloud_keypoint_detectors_, ambient_pointcloud, ambient_search_method, ambient_pointcloud_keypoints_out);
					computed_keypoints = true;
				}

				ambient_pointcloud->header.frame_id = map_frame_id_;
				if (applyCloudRegistration(tracking_recovery_matchers_, ambient_pointcloud, ambient_search_method, (ambient_pointcloud_keypoints_out->size() < (size_t)minimum_number_of_points_in_ambient_pointcloud_) ? ambient_pointcloud : ambient_pointcloud_keypoints_out, pose_corrections_out)) {
					pointcloud_pose_corrected_out = pose_corrections_out * pointcloud_pose_initial_guess;
					if (!postProcessCloudRegistration(pointcloud_pose_initial_guess, pointcloud_pose_corrected_out, post_process_cloud_registration_pose_corrections, pointcloud_time)) { return false; }
					pcl::transformPointCloudWithNormals(*ambient_pointcloud, *ambient_pointcloud, laserscan_to_pointcloud::tf_rosmsg_eigen_conversions::transformTF2ToTransform<double>(post_process_cloud_registration_pose_corrections));
					pcl::transformPointCloudWithNormals(*ambient_pointcloud_keypoints_out, *ambient_pointcloud_keypoints_out, laserscan_to_pointcloud::tf_rosmsg_eigen_conversions::transformTF2ToTransform<double>(post_process_cloud_registration_pose_corrections));
					pose_corrections_out = post_process_cloud_registration_pose_corrections * pose_corrections_out;
					pointcloud_pose_corrected_out = pose_corrections_out * pointcloud_pose_initial_guess;

					ROS_INFO("Successfully applied registration recovery");
					localization_times_msg_.pointcloud_registration_time += performance_timer.getElapsedTimeInMilliSec();

					if (ambient_pointcloud_integration) {
						pcl::transformPointCloudWithNormals(*ambient_pointcloud_integration, *ambient_pointcloud_integration, laserscan_to_pointcloud::tf_rosmsg_eigen_conversions::transformTF2ToTransform<double>(pose_corrections_out));
					}

					performance_timer.restart();
					applyAmbientPointCloudOutlierDetection(ambient_pointcloud_integration ? ambient_pointcloud_integration : ambient_pointcloud);
					applyReferencePointCloudOutlierDetection(ambient_search_method, reference_pointcloud_);
					localization_times_msg_.outlier_detection_time += performance_timer.getElapsedTimeInMilliSec();

					performance_timer.restart();
					applyCloudAnalysis(pointcloud_pose_corrected_out);
					localization_times_msg_.registered_points_angular_distribution_analysis_time += performance_timer.getElapsedTimeInMilliSec();

					performance_timer.restart();
					if (!applyTransformationValidators(transformation_validators_tracking_recovery_, pointcloud_pose_initial_guess, pointcloud_pose_corrected_out, outlier_percentage_, outlier_percentage_reference_pointcloud_)) {
						sensor_data_processing_status_ = PoseEstimationRejectedByTransformationValidators;
						return false;
					}
				} else {
					sensor_data_processing_status_ = FailedPoseEstimation;
					return false;
				}
			} else {
				sensor_data_processing_status_ = FailedPoseEstimation;
				return false;
			}
		}
	}

	if (last_pose_weighted_mean_filter_ > 0.0 && last_pose_weighted_mean_filter_ < 1.0) {
		pointcloud_pose_corrected_out.getOrigin().setInterpolate3(last_accepted_pose_base_link_to_map_.getOrigin(), pointcloud_pose_corrected_out.getOrigin(), last_pose_weighted_mean_filter_);
		pointcloud_pose_corrected_out.setRotation(tf2::slerp(last_accepted_pose_base_link_to_map_.getRotation(), pointcloud_pose_corrected_out.getRotation(), last_pose_weighted_mean_filter_));
	}

	pointcloud_pose_corrected_out.getRotation().normalize();
	pose_corrections_out.getRotation().normalize();

	localization_times_msg_.transformation_validators_time += performance_timer.getElapsedTimeInMilliSec();
	ambient_pointcloud->header.stamp = (std::uint64_t)(pointcloud_time.toNSec() / 1000.0);
	if (ambient_pointcloud_integration) {
		ambient_pointcloud_integration->header.stamp = (std::uint64_t)(pointcloud_time.toNSec() / 1000.0);
	}

	performance_timer.restart();
	if (registration_covariance_estimator_) {
		double opengl_matrix[16];
		pose_corrections_out.getOpenGLMatrix(opengl_matrix);
		Eigen::Matrix4d registration_corrections(opengl_matrix);

		if (registered_inliers_->size() > (size_t)minimum_number_of_points_in_ambient_pointcloud_) {
			typename pcl::search::KdTree<PointT>::Ptr registered_inliers_search_method(new pcl::search::KdTree<PointT>());
			registered_inliers_search_method->setInputCloud(ambient_pointcloud);
			registration_covariance_estimator_->computeRegistrationCovariance(registered_inliers_, registered_inliers_search_method, registration_corrections.cast<float>(),
					laserscan_to_pointcloud::tf_rosmsg_eigen_conversions::transformTF2ToTransform<float>(pointcloud_pose_corrected_out.inverse()), base_link_frame_id_, last_accepted_pose_covariance_);
		} else {
			registration_covariance_estimator_->computeRegistrationCovariance(ambient_pointcloud, ambient_search_method, registration_corrections.cast<float>(),
					laserscan_to_pointcloud::tf_rosmsg_eigen_conversions::transformTF2ToTransform<float>(pointcloud_pose_corrected_out.inverse()), base_link_frame_id_, last_accepted_pose_covariance_);
		}
	}
	localization_times_msg_.covariance_estimator_time = performance_timer.getElapsedTimeInMilliSec();

	last_accepted_pose_base_link_to_map_ = pointcloud_pose_corrected_out;
	last_accepted_pose_time_ = ros::Time::now();
	last_accepted_pose_valid_ = true;
	robot_initial_pose_available_ = true;
	received_external_initial_pose_estimation_ = false;
	pose_tracking_number_of_failed_registrations_since_last_valid_pose_ = 0;

	publishDetectedOutliers();
	publishDetectedInliers();

	if (ambient_pointcloud_integration) {
		ROS_DEBUG("Switching SLAM cloud");
		ambient_pointcloud = ambient_pointcloud_integration;
	}

	sensor_data_processing_status_ = SuccessfulPoseEstimation;
	return true;
}


template<typename PointT>
bool Localization<PointT>::updateReferencePointCloudWithAmbientPointCloud(typename pcl::PointCloud<PointT>::Ptr& pointcloud, typename pcl::PointCloud<PointT>::Ptr pointcloud_keypoints) {
	ROS_DEBUG_STREAM("Adding " << pointcloud->size() << " points to a reference cloud with " << reference_pointcloud_->size() << " points");

	*reference_pointcloud_ += *pointcloud;
	*reference_pointcloud_keypoints_ += *pointcloud_keypoints;

	if (use_incremental_map_update_) {
		localization_diagnostics_msg_.number_points_reference_pointcloud = reference_pointcloud_->size();
		localization_diagnostics_msg_.number_points_reference_pointcloud_after_filtering = reference_pointcloud_->size();
		localization_diagnostics_msg_.number_keypoints_reference_pointcloud = reference_pointcloud_keypoints_->size();
		reference_pointcloud_search_method_->setInputCloud(reference_pointcloud_);

		updateMatchersReferenceCloud();
		publishReferencePointCloud(pcl_conversions::fromPCL(pointcloud->header).stamp, true);

		return true;
	} else {
		return updateLocalizationPipelineWithNewReferenceCloud(pcl_conversions::fromPCL(pointcloud->header).stamp);
	}

	return false;
}
// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>   </Localization-functions>  <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<
// =============================================================================  </public-section>  ===========================================================================



// =============================================================================   <protected-section>   =======================================================================
// =============================================================================   </protected-section>  =======================================================================


} /* namespace dynamic_robot_localization */
