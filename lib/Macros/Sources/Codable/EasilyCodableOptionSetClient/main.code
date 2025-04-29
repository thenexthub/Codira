import EasilyCodableOptionSet

@EasilyCodableOptionSet struct MyOptionSet: OptionSet {
    var rawValue: Int
    static let optionOne = MyOptionSet(rawValue: 1 << 0)
    static let optionTwo: MyOptionSet = .init(rawValue: 1 << 1)
    static let optionThree: MyOptionSet = MyOptionSet(rawValue: 1 << 2)
    static let all: MyOptionSet = [.optionOne, .optionTwo, .optionThree]

    init(rawValue: Int) {
        self.rawValue = rawValue
    }
}
