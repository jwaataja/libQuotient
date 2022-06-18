// SPDX-FileCopyrightText: 2021 Carl Schwan <carlschwan@kde.org>
//
// SPDX-License-Identifier: LGPL-2.1-or-later

#include "e2ee/qolmsession.h"
#include "testolmsession.h"

using namespace Quotient;

std::pair<QOlmSessionPtr, QOlmSessionPtr> createSessionPair()
{
    QByteArray pickledAccountA("oTJQxRSwLqtIzXswxjRdBcCAUHe2er2rWh5Q2WI3lYoKIeZm3iJT6dxzYT0lHXnYVq0LaA3GoucKKwsM4b/PxKnHX92gHGG3tLaPqxICFUXDJelLQS2eflC+mKHrZan82C9NoA94FYB6FgFPUHKUesywZtmYtX/62OCENDhZnXnvRJoAnGDvx18BkSgyBtoBdruBWNUCF3DfH9CZ4E9j7OrLUAq/yKy6eMDVzMd6Vrt4zakjcOFaU7VLs3ftdDpdSl+Ip7KiCV76Q0w/Kh0e7Ak6mdbo1pijBdrVRj+LOcYIfbJ/NRh4DakLiSs1pGoEIrQ5F0FbedjSqFyCDQrECIZeHrwkmam8HQAJJpByKqow5JKBFxRBO0vv3Z9HPYCP/RELOCzNfEhZ7F+rYChsr30IAExNSQ3RHUX3zeKartE/NS+rxxagR9CKtvs1t8zL8u3IAny/Uf44kv+1M2mdV0NS2GASmC/al+SDHxi0JtApptp4Gbrd3AvbdYtBKCmtYRv/Za091adkN3S+evhwd6L59yllpaRWXnqKHIMHc65UcIoK1H5DBHorJ04CTPt044Sc7lAnV7qXnRU4hTAQsM21c04S82kqFaUv+9LmPSrmcT4VNPs7BW+ya6G9O/Tj3BZbEv90NbtfE6HccA/6ta4Yvcd+cSL3hohLrVEOdCKRKmaElcpuxWKhgIuLvOqv8s/dehC6AP+XwGX4+MlrdQ5NI24xBMVFeruxI13+0ZxBQegdKHaRqIojCKDUoVTtPs+Jsmdr4Se7/3gQGmQ9wpwlnE0FSs3sbWNMEgf0p4TtXdfUPVBITu5a6LSGvWrutGkoNJqqB+fgz3J2A8qaK68o3OFDRwAEecD96KIb93ih7Ip1Z1vPxFMEUuuJLHJP0SzchTdopSmRk9gcY24rVPB08NDWj6amdSQPKWzXld8Ls4K5PxBqsYCp6j2whA8jpGshqKpznrXgxx/+nULfwRjeyCnbIWkZESpf9CrcXfFqZpr0mtxYZBiFQvLDrBDAMpU5x1UtXp6UoKYcJhuKeiSBZw7M4TmI");
    QByteArray pickledAccountB("oTJQxRSwLqtIzXswxjRdBcQ4n5Sv9PV9W5ndeimOtk84WPmeV4u8LR5PmoS2VW9sEMJSajyZdEYxGqYJ2uYpRqfnARFfSxPI/pDhkdA6YAKlPMbxKknMhLQj1CPL3sXK514ChygOWW1ytebi98oWONrBuqzN7mm535ABBiLZ6K3zfIpZhuB9PQxaCsb+ARIyC0Yrw+LjI0dpONJiasI19ugsWD6rzGnfbLkcUTntGSmpz3sXszxm/mvgT2OJ+UCyGK82fkcs7FOffYi/KLZXxx5GEaPfURdD71RniXIdEN6+FbuD23HW1P5bYm3tX8nauyt0nU6MnVXFisnxisi6vzq2PATVoBptz3uqv4gWSCU76hEKzvYmHe3Rf/NoA349fEwhhwCmRazMo5fK6gNSEO0fYeOsTLrVFvtfiWQ7NtrTKVI6jigNliRNX/WLVu6jW26uMSe3p+MSIT0ipSPvxdzbsIQQe81ElRVJqu9rm7xuikY+s5EXA+YrCAgufCbbCyjTgzjVAujRDTP6JOpXTigitBkpoMOkkc0dYz4/vG9tGIatE1Nidy/dt57F7I82qB7NNoNxl1N7ie2E0FX2oci8LnerZP4nIB6OOX6I0k03k5LKA97tA9VK8t8uzASfHSbkGCtZm/2BViW7U2KOt3F9o1y0EntMWa1Buu+8Cv1NvprVeYervkiNai0+x5C3dsPJ3aiATB3auLJPtsjgjus8ZkBi3kWVhhZSq72HFcvsUpgrWwWj7/kXRMF+kMNr5Rf5ImmNSHtdv91aoMTr1WAkKXfl/+oQTsmp03MY8iqPPA9KYhDtnpJgtIbefBiY7lTqSz2p45ieJxRPOKm9q4TAoxp7Dd74MPVn669sRBsRhRb+ZVkFLueBzbuzUbCOxL50hf13MBMsI9a+kQlaMzerYDnGcXje/IbLcLCnCoFbYII0H2o1hKO3pCbSVx0DHgKdALXxzq2ssLBbD/PIwRDUHT8CFOJtyIGYKmLee2xB1KKXN22sco6FJ78EoNsVBSdpoje7DfAp4y8/BMDG11lgVg+LWM6C");
    auto accountA = QOlmAccount("accountA:foo.com", "Device1UserA");
    accountA.unpickle(pickledAccountA, Unencrypted{});
    auto accountB = QOlmAccount("accountB:foo.com", "Device1UserB");
    accountB.unpickle(pickledAccountB, Unencrypted{});

    const QByteArray identityKeyA("qIEr3TWcJQt4CP8QoKKJcCaukByIOpgh6erBkhLEa2o");
    const QByteArray oneTimeKeyA("WzsbsjD85iB1R32iWxfJdwkgmdz29ClMbJSJziECYwk");
    const QByteArray identityKeyB("q/YhJtog/5VHCAS9rM9uUf6AaFk1yPe4GYuyUOXyQCg");
    const QByteArray oneTimeKeyB("oWvzryma+B2onYjo3hM6A3Mgo/Yepm8HvgSvwZMTnjQ");
    auto outbound =
        accountA.createOutboundSession(identityKeyB, oneTimeKeyB).value();

    const auto preKey = outbound->encrypt(""); // Payload does not matter for PreKey

    if (preKey.type() != QOlmMessage::PreKey) {
        // We can't call QFail here because it's an helper function returning a value
        throw "Wrong first message type received, can't create session";
    }
    auto inbound = accountB.createInboundSession(preKey).value();
    return { std::move(inbound), std::move(outbound) };
}

void TestOlmSession::olmOutboundSessionCreation()
{
    const auto [_, outboundSession] = createSessionPair();
    QCOMPARE(0, outboundSession->hasReceivedMessage());
}

void TestOlmSession::olmEncryptDecrypt()
{
    const auto [inboundSession, outboundSession] = createSessionPair();
    const auto encrypted = outboundSession->encrypt("Hello world!");
    if (encrypted.type() == QOlmMessage::PreKey) {
        QOlmMessage m(encrypted); // clone
        QVERIFY(inboundSession->matchesInboundSession(m));
    }

    const auto decrypted = inboundSession->decrypt(encrypted).value();

    QCOMPARE(decrypted, "Hello world!");
}

void TestOlmSession::correctSessionOrdering()
{
    // n0W5IJ2ZmaI9FxKRj/wohUQ6WEU0SfoKsgKKHsr4VbM
    auto session1 = QOlmSession::unpickle("md9iaGsws2Cf4uwWglGCurvhhKfgvIlHYThGA+6m40eP1CDgtLRgWTrinXfLqu2cG0NKWby2JiDGIIlx/R91ZgqFkGzq+Ltc7pR6RhZlYaAoo+y7f/g/HinPD9mIvJ5+NEt0uI5X5G7pMJkZlodxpqZHJwmuQGuVyqs19BaqqEdAxu2NzBgOxKCydHW1cBCapBYzg/c3BMtuHegKiPOgG3RHYg4NHHkcY4Aq7+w+pOFz0LCZF4z1nt55f2xNL/U4Gv698aiNceBWBo0Pf21fBsxRFZH0yURaF+L4MxRGJK/BDVaFll0IkF3CaqglST58uevCl5va7VMlf8AJHMVxcwSo34cma6iIMO5NNRaQLP9gGsXR7O0oqal8RjgfwFWYV2LGlkStmqwah0UvFBr2KTEzL+7bzWwRWmyI9NmgLOWte7fzKyu0/9TUq9bfVNzKxMuU23kFwmlpD0CWdlpUg6rW7IX13fMi+46SgpxSXsGnuHHYJWM8lboME5XZV22VHuxeOCbJDdZa0KpPY4QTn04P7AWMYyzhRBZ3mKg7GumGBW1uuUdZFL4PHY6Z+CHzyvEm/brcLF2F8RgY5la60dgPrTUM4BSuZpSSS9yJKBiEuwPCTWWlJ6Jh9J5WDrtZu9F83ehzUZit2zP8qpxmJOZbMA3kBVnHbRVFfJX4zg9bnkgV7Fk3pTL4sy+NLf3jh5C9vSfGt0wRx+9mnhxnDiG9mrKKBuXE7it+7DQ6lNlT2fdwmUrG3g1Lpm1DQ1mVIS8X4tdeCjUjb+JzPQfjkEcAk/nq9O32YxJI2s4TFw3Tql3O05+Adh6HumgCzn0lG2rdNz7Bf38bmw1jSrrP78u3waxGxF9Y6sEPQni2srSqcr9m1YJxEGElO6YeR1WgoFVY6TwCqerYYL9/GVl9Ectab1R/C9/wOjfD5t5oazQyFTSvwZVInawTz2d5WPW6tvS0CVHrzr56QzGhKaeho/bjiBFOG74oiM/yvkX61RBXowPR5zlzRRqBWKcCyMcg4upeUxWM8yyMPjPrEs3P4IataLjpBMyJaYnlAW/vIYD2tko6WhhLKJLBBXazH4mqZowXvWsh4wPG0COlYW7eeI2zUFp0SCzJvUoVvBaxoyt92BxtxRIA2zsK+EPg1gnzZAuWBwe4PJrw0XPjLaxBq9fPN11+PsrBBr29Pn9jzdXNmL4I4AHsQnyTh7nNUA/y2EomBzscHxp2JhXpoVgZ5c+huk20eJTb", Unencrypted{}).value();
    // +9pHJhP3K4E5/2m8PYBPLh8pS9CJodwUOh8yz3mnmw0
    auto session2 = QOlmSession::unpickle("md9iaGsws2Cf4uwWglGCurvhhKfgvIlHYThGA+6m40eP1CDgtLRgWTrinXfLqu2cG0NKWby2JiDGIIlx/R91ZgqFkGzq+Ltc7pR6RhZlYaAoo+y7f/g/HinPD9mIvJ5+NEt0uI5X5G7pMJkZlodxpqZHJwmuQGuVyqs19BaqqEdAxu2NzBgOxKCydHW1cBCapBYzg/c3BMtuHegKiPOgG9wKSYjAVGCJ7+u10CuzskL5Zub0I9QKL2yGSi3b35hqKyizzrtMzCffP90p2fXxUZHGMiK/ggDIlgB96xaduGGeYvWp/Kv9KAb/ZEhWQnau8KbyTcN+np0RxSgsSuR8vqOIXCtIzhi55j5uIg2L0fxkLr9JG7Y6qr/ixbWA97lRKjzd6avzKcGuIN7H7lJdfXt1bt+5e7kZRaUQWTz+6NRxnT7O7+nMaqp+/NAIXcxShz3hd+bpck7Ulm+rz5EJnCpP1/agbvmc1zlWG3gjgVlAMo7EcHu4dtehSp2h1Ncft0JjJk1kFHvOrUwTH2d5qe+ozKJyX937/Caqgz1bmJDVqapjXpBQREtxpWxoLxql8PhSznW6ifhpmWlqUL0DONnSb4i/5BSIJ09ith26xtVv0q9bxVjP4bCdu1wVMaGWfbSDwNqU5NxJO6OzOjLz5+XPPpUc5pp76YDp2NFJF2+n/n4gp0/fM8ZYtewJE/6nOTgD0mcGfiJ7/1S+csCMjWttrrfdKjoUAiJe/4b4poSt5P71xzt2GRKjBPYlO7pawXRgl7iaOwvbkFDSl1qU68Yqxsm4HqwxldGPlwsV/Ooy8nEiZFSqUiYqbex3zJbmYVZchujJQhp1FTbtJ/qrbWFPuqQt4QWALjrJtDvNU8soHHoI4u/xViXE7dpX7hXGTCtzb/5KnZBP+8CXKaF17y4xwRDtJzzATmNPypEkqj1Z/LuNTSBs5UffqycG8rP0kmiRzVqrstxrdmr9oU6nJWM+QzHGjAyYEMP+4pixwkANalHkyXbLy9rd/YphQOzK7ggSmwcs7ug/yfID/gNxMMRpdZ4pFjUSLMa5PcS5VLRxAOfC0GXFGFqZt1LCIuNlY5fFv318t1zkfQL3HI8JNB3NGKNdRUZUZZYzr3PyVP0lpcPXsflpvEV0DDz18KK6tsKg8W4Ql+iC+0HGWK/6nqZDJWiPgTnnFLVNs5xJS0+ewfLRrd8Xh67xzFmEO4OrUPjsuJ5pnBU", Unencrypted {}).value();
    // MC7n8hX1l7WlC2/WJGHZinMocgiBZa4vwGAOredb/ME
    auto session3 = QOlmSession::unpickle("md9iaGsws2Cf4uwWglGCurvhhKfgvIlHYThGA+6m40eP1CDgtLRgWTrinXfLqu2cG0NKWby2JiDGIIlx/R91ZgqFkGzq+Ltc7pR6RhZlYaAoo+y7f/g/HinPD9mIvJ5+NEt0uI5X5G7pMJkZlodxpqZHJwmuQGuVyqs19BaqqEdAxu2NzBgOxKCydHW1cBCapBYzg/c3BMtuHegKiPOgG691Cgd6+/ul3ExulJYbau1nQtIAOxJ89+8Ro53A2cdVDprASVxidGT2N7spR2e1eYa5AGh3pYKYfLik0oePLuQeNAf99MaJrA4++fNypYPPGLQb30W/k8b+MTzO/LkY/70ElrUC49NkNELwXalC0BwHdNGUEhvZCOZkzSL9qZ9MHd+N/6XU2qp12tH6v37zf3zyF4E+B2jjiw5VGuDMumXX1utf/S5XmC1NOorJ8Trx2C8Svwy9Pycza1Cm4xNdz4V4StTtwqM5fy+NJSFiptazjujisfFyeVzjz+IizlxcbPSwqI4WK2lLMXAVfWjiZ9GwlecyPgvbrZfzqq977wuNiBe0M6B1l9qW+1g0bHt1mxJk5aVWOSLi0s0TvGbM/uXQRoaNXJBvCZ/7NkhMJ6oGkghYVBhhowTm8lOERaRHVK0CTilm70BoU7BZCtgqcWmHOe/+G8nBUzmVNdBCOul7yX5uVSpqk5akn6boeG90ShcVMlLp46Ctl4NP4aEzkOfUb6y63Tm8OcXw+T5yHDAD7wZC7sSU6TmYMd6MvmrqkOW4mWm3UwqL3mAurElIMi99gQlD2W8OrmJlzVNKqK00YdmM8v+JZ8Smlf9YNOXSD6d4nXE/nvUGygxUSEdAwLuQH/su2fQE5uhtoxZuAbdLkkJqbroaMzrBOueGtXrwT51oNUqMDKoin1suFGwvtJrV8NzqAkNyexh/Ju5ca5Z9aUzw/K7px/aSDqo2Pw3Vv5L8BUyKhWxzQqSeGpuvGEkV7eL5RlvHvF12FWN6U1F9K94uFXb1dMWstek/g/06z/EN2elrIK1T1Vdc71AkMFTIlt87bg7jI/A0VxnHH3x6i/dhFnzvUHp6hGp2IZgga1LUHN8mc2V+aJxZ1nr2bvHYYfctePzt4eaPQbeY4gANyXoF3auMrarsDRSZ7n3Wf0LbBuxWU99MoIDJZ14hZKpJA8GJLKKwkQUDXfbBm3FAxdoehgYzJd1UmV9MGQ1ShOEeiRCU/Pw", Unencrypted{}).value();

    const auto session1Id = session1->sessionId();
    const auto session2Id = session2->sessionId();
    const auto session3Id = session3->sessionId();

    std::vector<QOlmSessionPtr> sessionList;
    sessionList.push_back(std::move(session1));
    sessionList.push_back(std::move(session2));
    sessionList.push_back(std::move(session3));

    std::sort(sessionList.begin(), sessionList.end());
    QCOMPARE(sessionList[0]->sessionId(), session2Id);
    QCOMPARE(sessionList[1]->sessionId(), session3Id);
    QCOMPARE(sessionList[2]->sessionId(), session1Id);
}

QTEST_GUILESS_MAIN(TestOlmSession)
