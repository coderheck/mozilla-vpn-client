/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/. */

import QtQuick 2.5
import QtQuick.Layouts 1.14

import Mozilla.Shared 1.0
import Mozilla.VPN 1.0
import components 0.1
import components.forms 0.1
import components.inAppAuth 0.1

MZInAppAuthenticationBase {
    id: authUnblockCodeNeeded

    _telemetryScreenId: "enter_unblock_code"
    _viewObjectName: "authUnblockCodeNeeded"
    _menuButtonImageSource: "qrc:/nebula/resources/close-dark.svg"
    _menuButtonOnClick: () => {
        if (isReauthFlow) {
            Glean.interaction.authenticationAborted.record({
                screen: _telemetryScreenId,
                action: "select",
                element_id: "close",
            });

            cancelAuthenticationFlow();
        } else {
            Glean.interaction.authenticationInappStep.record({
                screen: _telemetryScreenId,
                action: "select",
                element_id: "close",
            });

            MZAuthInApp.reset();
        }
    }
    _menuButtonAccessibleName: MZI18n.GlobalClose
    _headlineText: MZI18n.InAppAuthVerificationCodeTitle
    _subtitleText: MZI18n.InAppAuthEmailVerificationDescription
    _imgSource: "qrc:/nebula/resources/verification-code.svg"

    _inputs: MZInAppAuthenticationInputs {
        objectName: "authUnblockCodeNeeded"
        _telemetryScreenId: authUnblockCodeNeeded._telemetryScreenId
        _buttonTelemetryId: "verify"
        _buttonEnabled: MZAuthInApp.state === MZAuthInApp.StateUnblockCodeNeeded && activeInput().text.length === MZAuthInApp.unblockCodeLength && !activeInput().hasError
        _buttonOnClicked: (inputText) => { MZAuthInApp.verifyUnblockCode(inputText) }
        _buttonText: MZI18n.InAppAuthVerifySecurityCodeButton
        _inputMethodHints: Qt.ImhNone
        _inputPlaceholderText: MZI18n.InAppAuthUnblockCodeInputPlaceholder
    }

    _footerContent: Column {
        Layout.alignment: Qt.AlignHCenter
        spacing: MZTheme.theme.windowMargin

        MZLinkButton {
            objectName: _viewObjectName + "-resendCode"
            labelText: MZI18n.InAppAuthResendCodeLink
            anchors.horizontalCenter: parent.horizontalCenter
            onClicked: {
                 Glean.interaction.authVerificationCodeResend.record({
                    screen: _telemetryScreenId,
                    action: "select",
                    element_id: "resend_code",
                });

                MZAuthInApp.resendUnblockCodeEmail();
                MZErrorHandler.requestAlert(MZErrorHandler.AuthCodeSentAlert);

            }
        }
        MZCancelButton {
            objectName: _viewObjectName + "-cancel"
            anchors.horizontalCenter: parent.horizontalCenter
            onClicked: {
                Glean.interaction.authenticationAborted.record({
                    screen: _telemetryScreenId,
                    action: "select",
                    element_id: "cancel",
                });

                if (isReauthFlow) {
                    cancelAuthenticationFlow();
                } else {
                    VPN.cancelAuthentication();
                }
            }
        }
    }

    Component.onCompleted: {
        Glean.impression.authenticationInappStep.record({
            screen: _telemetryScreenId,
            action: "impression"
        });
    }
}
