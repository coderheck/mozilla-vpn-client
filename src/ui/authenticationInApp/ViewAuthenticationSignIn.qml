/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/. */

import QtQuick 2.5
import QtQuick.Layouts 1.14

import Mozilla.Shared 1.0
import Mozilla.VPN 1.0
import components 0.1
import components.inAppAuth 0.1

MZInAppAuthenticationBase {
    id: authSignIn

    _telemetryScreenId: "enter_password"

    states:[
        State {
            when: isReauthFlow

            PropertyChanges {
                target: authSignIn

                _changeEmailLinkVisible: false
                _subtitleText: MZI18n.InAppAuthReauthSignInSubtitle2
            }

            PropertyChanges {
                target: disclaimersLoader

                source: ""
            }

            PropertyChanges {
                target: authInputs

                _buttonText: MZI18n.DeleteAccountAuthButtonLabel
            }
        }
    ]

    _changeEmailLinkVisible: true
    _viewObjectName: "authSignIn"
    _menuButtonImageSource: "qrc:/nebula/resources/back.svg"
    _menuButtonImageMirror: MZLocalizer.isRightToLeft
    _menuButtonOnClick: () => {
        if (isReauthFlow) {
            Glean.interaction.authenticationAborted.record({
                screen: authSignIn._telemetryScreenId,
                action: "select",
                element_id: "back_arrow",
            });

            cancelAuthenticationFlow();
        } else {
            Glean.interaction.authenticationInappStep.record({
                screen: authSignIn._telemetryScreenId,
                action: "select",
                element_id: "back_arrow",
            });

            MZAuthInApp.reset();
        }
    }
    _menuButtonAccessibleName: MZI18n.GlobalGoBack
    _headlineText: MZAuthInApp.emailAddress
    _subtitleText: MZI18n.InAppAuthSignInSubtitle2
    _imgSource: "qrc:/nebula/resources/avatar.svg"
    _inputLabel: MZI18n.InAppAuthPasswordInputLabel

    _inputs: MZInAppAuthenticationInputs {
        objectName: "authSignIn"
        id: authInputs

        _telemetryScreenId: authSignIn._telemetryScreenId
        _buttonTelemetryId: "sign_in"

        _buttonEnabled: MZAuthInApp.state === MZAuthInApp.StateSignIn && !activeInput().hasError
        _buttonOnClicked: (inputText) => {
             MZAuthInApp.setPassword(inputText);
             MZAuthInApp.signIn();
         }
        _buttonText: MZI18n.InAppAuthSignInButton
        _inputPlaceholderText: MZI18n.InAppAuthPasswordInputPlaceholder
    }

    _disclaimers: Loader {
        id: disclaimersLoader

        Layout.alignment: Qt.AlignHCenter
        source: "qrc:/nebula/components/inAppAuth/MZInAppAuthenticationLegalDisclaimer.qml"
    }

    _footerContent: Column {
        Layout.alignment: Qt.AlignHCenter
        Layout.preferredWidth: parent.width
        spacing: MZTheme.theme.windowMargin

        MZLinkButton {
            objectName: _viewObjectName + "-forgotPassword"
            labelText: MZI18n.InAppAuthForgotPasswordLink
            anchors.horizontalCenter: parent.horizontalCenter
            onClicked: {
                Glean.interaction.authPasswordResetRequest.record({
                    screen: _telemetryScreenId,
                    action: "select",
                    element_id: "forgot_your_password",
                });

                MZUrlOpener.openUrlLabel("forgotPassword")
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
