// Copyright © 2017-2019 Trust Wallet.
//
// This file is part of Trust. The full Trust copyright notice, including
// terms governing use, modification, and redistribution, is contained in the
// file LICENSE at the root of the source code distribution tree.

import TrustWalletCore
import XCTest

extension String {
    public func indexOf<T>(stuff: T) -> Int where T: StringProtocol {
        let r = range(of: stuff)
        guard let rr = r else {
            return -1
        }
        return rr.lowerBound.encodedOffset
    }
}

class OntologyTests: XCTestCase {
    
    func testSignOngBalanceOf() {
        
        let input = TWOntologySigningInput.with {
            $0.contract = "ONG"
            $0.method = "balanceOf"
            $0.queryAddress = "ANDfjwrUroaVtvBguDtrWKRMyxFwvVwnZD"
        }
        
        let output = OntologySigner.encode(input: input)
        let result = output.encoded.hexString
        
        XCTAssertEqual(86, result.indexOf(stuff: "1446b1a18af6b7c9f8a4602f9f73eeb3030f0c29b70962616c616e63654f661400000000000000000000000000000000000000020068164f6e746f6c6f67792e4e61746976652e496e766f6b65"))
    }
    
    func testSignOntBalanceOf() {
        
        let input = TWOntologySigningInput.with {
            $0.contract = "ONT"
            $0.method = "balanceOf"
            $0.queryAddress = "ANDfjwrUroaVtvBguDtrWKRMyxFwvVwnZD"
        }
        
        let output = OntologySigner.encode(input: input)
        let result = output.encoded.hexString
        
        XCTAssertEqual(0, result.indexOf(stuff: "00d1"))
        XCTAssertEqual(86, result.indexOf(stuff: "1446b1a18af6b7c9f8a4602f9f73eeb3030f0c29b70962616c616e63654f66140000000000000000000000000000000000000001"))
    }
    
    func testSignOntTransfer() {
        
        let input = TWOntologySigningInput.with {
            $0.contract = "ONT"
            $0.method = "transfer"
            $0.fromPrivateKey = "4646464646464646464646464646464646464646464646464646464646464646"
            $0.toAddress = "Af1n2cZHhMZumNqKgw9sfCNoTWu9de4NDn"
            $0.amount = 1
            $0.gasPrice = 500
            $0.gasLimit = 20000
            $0.payerPrivateKey = "4646464646464646464646464646464646464646464646464646464646464652"S
        }
        
        let output = OntologySigner.encode(input: input)
        let result = output.encoded.hexString
        
        let verifyPosition1 = result.indexOf(stuff: "21031bec1250aa8f78275f99a6663688f31085848d0ed92f1203e447125f927b7486ac")
        let verifyPosition2 = result.indexOf(stuff: "2103d9fd62df332403d9114f3fa3da0d5aec9dfa42948c2f50738d52470469a1a1eeac")
        
        XCTAssertEqual(724, result.count)
        XCTAssertEqual(450, verifyPosition1)
        XCTAssertEqual(654, verifyPosition2)
    }
    
}
