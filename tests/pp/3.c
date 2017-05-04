#define m1(x) x

m1(5)

m1((5 + 2))

m1(plus(5, 3))

m1() 10

m1(2 + 2 +) 10


#define m2(x) x + x
m2(5)

#define m3(x, y) x * y
m3(5, 10)
m3(2 + 2, 3 + 3)

#define m4(x, y) x + y + TWO
m4(5, 10)

#define plus(x, y) x * y + plus(x, y)
plus(2, 3)


#define m6(x, ...) x + __VA_ARGS__
m6(2, 18)
plus(m6(2, 18, 5))



/*
#define plus(x, y)  minus(x, y)
#define minus(x, y) plus(x, y)
    expect(31, plus(30, 1));
    expect(29, minus(30, 1));

    // This is not a function-like macro.
#define m7 (0) + 1
    expect(1, m7);

#define m8(x, y) x ## y
    expect(2, m8(TW, O));
    expect(0, m8(ZERO,));
    expect(8, 1 m8(<, <) 3);
    expectf(.123, m8(., 123));
    expect('a', m8(L, 'a'));
    expect('a', m8(U, 'a'));
    expect('a', m8(u, 'a'));
    expect_string(L"abc", m8(L, "abc"));
    expect_string(U"abc", m8(U, "abc"));
    expect_string(u"abc", m8(u, "abc"));
    expect_string(u8"abc", m8(u8, "abc"));

#define m9(x, y, z) x y + z
    expect(8, m9(1,, 7));

#define m10(x) x ## x
    expect_string("a", "a" m10());

#define hash_hash # ## #
#define mkstr(a) # a
#define in_between(a) mkstr(a)
#define join(c, d) in_between(c hash_hash d)
    expect_string("x ## y", join(x, y));

    int m14 = 67;
#define m14(x) x
    expect(67, m14);
    expect(67, m14(m14));

    int a = 68;
#define glue(x, y) x ## y
    glue(a+, +);
    expect(69, a);

#define identity(x) stringify(x)
    expect_string("aa A B aa C", identity(m10(a) A B m10(a) C));

#define identity2(x) stringify(z ## x)
    expect_string("zA m10(a) A B m10(a) C", identity2(A m10(a) A B m10(a) C));

#define m15(x) x x
    expect_string("a a", identity(m15(a)));

#define m16(x) (x,x)
    expect_string("(a,a)", identity(m16(a)));

#define m17(x) stringify(.x . x)
    expect_string(".3 . 3", m17(3));

    */