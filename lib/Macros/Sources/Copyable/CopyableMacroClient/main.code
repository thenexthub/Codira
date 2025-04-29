import CopyableMacro

@Copyable
final class Sample {
    var x: Int
    let y: Double

    init(x: Int, y: Double) {
        self.x = x
        self.y = y
    }
}

let sample = Sample(x: 1, y: 2)

@Copyable
struct StructSample {
    var firstProp: Bool
    let secondProp: String
    //let t: String? = nil
    var fourth: String {
        secondProp
    }
    let fifth: [Int]
    
}

let structSample = StructSample(firstProp: true, secondProp: "secondProp", fifth: [0, 1])

print(sample)
