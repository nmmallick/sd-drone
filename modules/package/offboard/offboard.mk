################################################################################
#
# offboard
#
################################################################################
OFFBOARD_SITE= "$(BR2_EXERTNAL_SD_DRONE)/modules/offboard"
OFFBOARD_METHOD = local
OFFBOARD_INSTALL_STAGING = NO
OFFBOARD_INSTALL_TARGET = YES
OFFBOARD_DEPENDENCIES = mavsdk

$(eval $(cmake-package))

