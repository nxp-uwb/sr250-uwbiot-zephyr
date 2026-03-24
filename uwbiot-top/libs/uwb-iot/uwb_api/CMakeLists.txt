# Copyright 2020,2022-2024 NXP
#
# NXP Proprietary. This software is owned or controlled by NXP and may only
# be used strictly in accordance with the applicable license terms.  By
# expressly accepting such terms or by downloading, installing, activating
# and/or otherwise using the software, you are agreeing that you have read,
# and that you agree to comply with and are bound by, such license terms.  If
# you do not agree to be bound by the applicable license terms, then you may
# not retain, install, activate or otherwise use the software.
#
IF(UWBIOT_OS_NATIVE)
    PROJECT(uwb_${UWBIOT_UWBD})
ELSE()
    PROJECT(uwb_api)
ENDIF()

SET(
    UWB_VERSION
    ${UWBIOTVER_STR_MAJOR}.${UWBIOTVER_STR_MINOR}.${UWBIOTVER_STR_DEV}
)

SET (UWBD_FW_DIR
    binaries/UWBD
)
SET(UWB_FW_SUBDIR ${UWBIOT_UWBD})

IF(UWBIOT_UWBD_SR250)
    SET(UWB_FW_SUBDIR SR200T)
ENDIF()

MESSAGE(
    STATUS
        "UWB_FW_SUBDIR:${UWB_FW_SUBDIR}"
)

FILE(
    GLOB
    source_files
    ${CMAKE_SOURCE_DIR}/${UWBD_FW_DIR}/${UWB_FW_SUBDIR}/*.h
    ${CMAKE_SOURCE_DIR}/${UWBD_FW_DIR}/${UWB_FW_SUBDIR}/*.md
    fira_generic/*.h
    fira_generic/*.c
    proprietary/*.h
    proprietary/*.c
    proprietary/${UWB_SUBDIR}/*.h
    proprietary/${UWB_SUBDIR}/*.c
    ../adaptation/UwbAdaptation.c
)

IF((UWBIOT_UWBD_SR150)
    AND SSS_HAVE_APPLET_SE051_UWB
)
    FILE(
        GLOB
        se_files
        secure_element/SE05XX/state_machine/*.c
        secure_element/SE05XX/state_machine/*.h
        secure_element/SE05XX/wrappers/*.c
        secure_element/SE05XX/wrappers/*.h
    )
ENDIF()

IF(UWBIOT_SESN_SNXXX AND NOT(UWBIOT_SESN_P71))
    LIST(
        APPEND
        source_files
        secure_element/SNXXX/UwbSeApi.h
        secure_element/SNXXX/UwbSeApi.c
    )
ENDIF()

IF(UWBIOT_UWBD_SR1XXT_SR2XXT)
    FILE(
        GLOB
        prop_files
        proprietary/SRXXX/*.h
        proprietary/SRXXX/*.c
        proprietary/SRXXX/${UWB_SUBDIR}/*.h
        proprietary/SRXXX/${UWB_SUBDIR}/*.c
    )
ENDIF()



IF(UWBIOT_OS_NATIVE)
    ADD_LIBRARY(
        ${PROJECT_NAME} SHARED
        ${source_files}
        ${se_files}
        ${prop_files}
    )
ELSE()
    ADD_LIBRARY(
        ${PROJECT_NAME}
        ${source_files}
        ${se_files}
        ${prop_files}
    )
ENDIF()

SET_TARGET_PROPERTIES(
    ${PROJECT_NAME}
    PROPERTIES
        VERSION ${UWB_VERSION}
        SOVERSION
        ${UWBIOTVER_STR_MAJOR}
)

TARGET_INCLUDE_DIRECTORIES(
    ${PROJECT_NAME}

    PUBLIC fira_generic
    PUBLIC proprietary
    PUBLIC proprietary/${UWB_SUBDIR}
    PUBLIC
    ${CMAKE_SOURCE_DIR}/${UWBD_FW_DIR}/${UWB_FW_SUBDIR}
    PUBLIC
    ${CMAKE_SOURCE_DIR}/firmware_images/${UWB_FW_SUBDIR}
)

IF(
    UWBIOT_UWBD_SR100T
    OR UWBIOT_UWBD_SR100S
)
    TARGET_INCLUDE_DIRECTORIES(
        ${PROJECT_NAME}
        PUBLIC
        ${CMAKE_SOURCE_DIR}/${UWBD_FW_DIR}/SR100T
        PUBLIC
        ${CMAKE_SOURCE_DIR}/firmware_images/SR100
    )
ENDIF()

IF(UWBIOT_UWBD_SR1XXT)
    TARGET_INCLUDE_DIRECTORIES(
        ${PROJECT_NAME}
        PUBLIC
        ${CMAKE_SOURCE_DIR}/${UWBD_FW_DIR}/SR1XX
        PUBLIC
        ${CMAKE_SOURCE_DIR}/firmware_images/SR1XX
        PUBLIC
        ${CMAKE_SOURCE_DIR}/firmware_images/SR1XX/Prod_ROW
        PUBLIC
        ${CMAKE_SOURCE_DIR}/firmware_images/SR100/Rhodes_Dev
        PUBLIC
        ${CMAKE_SOURCE_DIR}/firmware_images/SR100/ROW_PROD
        PUBLIC proprietary/SRXXX/${UWB_SUBDIR}
        PUBLIC proprietary/SRXXX/
    )
ENDIF()

IF(UWBIOT_UWBD_SR2XXT)
    TARGET_INCLUDE_DIRECTORIES(
        ${PROJECT_NAME}
        PUBLIC
        ${CMAKE_SOURCE_DIR}/${UWBD_FW_DIR}/SR2XX
        PUBLIC
        ${CMAKE_SOURCE_DIR}/firmware_images/SR2XX
        PUBLIC
        ${CMAKE_SOURCE_DIR}/firmware_images/SR2XX/Prod_ROW
        PUBLIC proprietary/SRXXX/${UWB_SUBDIR}
        PUBLIC proprietary/SRXXX/
    )
ENDIF()

IF((UWBIOT_UWBD_SR150)
    AND SSS_HAVE_APPLET_SE051_UWB
)
    TARGET_INCLUDE_DIRECTORIES(
        ${PROJECT_NAME}
        PUBLIC secure_element/SE05XX/state_machine/
        PUBLIC secure_element/SE05XX/wrappers/
    )
    TARGET_LINK_LIBRARIES(
        ${PROJECT_NAME}
        se051w_fira
        ex_common
    )
ENDIF()

IF(UWBIOT_SESN_SNXXX AND NOT(UWBIOT_SESN_P71))
    TARGET_INCLUDE_DIRECTORIES(
        ${PROJECT_NAME}
        PUBLIC secure_element/SNXXX
    )
    TARGET_LINK_LIBRARIES(
        ${PROJECT_NAME} nfc_core
    )
ENDIF()

TARGET_LINK_LIBRARIES(
    ${PROJECT_NAME}
    uci_core
    uwb_core
)

IF(UWBIOT_HOST_LINUX_LIKE)
    INSTALL(
        FILES
        fira_generic/UwbApi.h
        fira_generic/UwbApi_Types.h
        fira_generic/UwbApi_Internal.h
        fira_generic/PrintUtility.h
        proprietary/SRXXX/UwbApi_Types_Proprietary.h
        proprietary/SRXXX/UwbApi_Proprietary.h
        proprietary/SRXXX/PrintUtility_Proprietary.h
        proprietary/SRXXX/UwbApi_Types_RfTest.h
        DESTINATION
        ${UWBIOT_INSTALL_INC_DIR}
    )

    IF(UWBIOT_UWBD_SR1XXT)
        INSTALL(
            FILES
            proprietary/SRXXX/SR1XX/UwbApi_Proprietary_Extended.h
            proprietary/SRXXX/SR1XX/UwbApi_Proprietary_Fm.h
            DESTINATION
            ${UWBIOT_INSTALL_INC_DIR}
        )
    ENDIF()

    IF(SSS_HAVE_APPLET_SE051_UWB)
        INSTALL(
            FILES
            #secure_element/SE05XX/*.c
            proprietary/SRXXX/SR1XX/UwbApi_Types_Proprietary.h
            secure_element/SE05XX/state_machine/*.h
            secure_element/SE05XX/wrappers/*.h
            DESTINATION
            ${UWBIOT_INSTALL_INC_DIR}
        )
    ENDIF()

    UWBIOT_INSTALL_LIBRARY(
        ${PROJECT_NAME}
    )
ENDIF()
