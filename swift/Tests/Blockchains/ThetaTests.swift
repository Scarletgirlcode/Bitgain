// Copyright © 2017-2019 Trust Wallet.
//
// This file is part of Trust. The full Trust copyright notice, including
// terms governing use, modification, and redistribution, is contained in the
// file LICENSE at the root of the source code distribution tree.

import XCTest
import TrustWalletCore

class ThetaTests: XCTestCase {
    func testSigner() {
        let input = TW_Theta_Proto_SigningInput.with {
            $0.chainId = "privatenet"
            $0.toAddress = "0x9F1233798E905E173560071255140b4A8aBd3Ec6"
            $0.thetaAmount = Data(hexString: "0a")!
            $0.tfuelAmount = Data(hexString: "14")!
            $0.sequence = 1
            $0.fee = Data(hexString: "e8d4a51000")!
            $0.privateKey = Data(hexString: "0x93a90ea508331dfdf27fb79757d4250b4e84954927ba0073cd67454ac432c737")!
        }

        let output = ThetaSigner.sign(input: input)

        XCTAssertEqual(output.hexString, "5190868498d587d074d57298f41853d0109d997f15ddf617f471eb8cbb7fff267cb8fe9134ccdef053ec7cabd18070325c9c436efe1abbacd14eb7561d3fc10501")
    }
}
