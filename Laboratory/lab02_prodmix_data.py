from dataclasses import dataclass


@dataclass(frozen=True)
class ProdMixData:
    model_name: str
    var_type: str  # type of J variables
    J: list[str]  # products
    I: list[str]  # resources
    D: dict[str, float]  # availability of resource i in I
    P: dict[str, float]  # profit of product j in J
    Q: dict[
        tuple[str, str], float
    ]  # amount of resource i in I required for each unit of j in J


farmer = ProdMixData(
    model_name="prodmix_farmer",
    var_type="cont",
    J=["tom", "pot"],
    I=["land", "tseeds", "ptub", "fert"],
    D={"land": 11, "tseeds": 70, "ptub": 18, "fert": 145},
    P={"tom": 6000, "pot": 7000},
    Q={
        ("land", "tom"): 1,
        ("land", "pot"): 1,
        ("tseeds", "tom"): 7,
        ("tseeds", "pot"): 0,
        ("ptub", "tom"): 0,
        ("ptub", "pot"): 3,
        ("fert", "tom"): 10,
        ("fert", "pot"): 20,
    },
)


perfumes = ProdMixData(
    model_name="prodmix_perfumes",
    var_type="cont",
    J=["one", "two"],
    I=["rose", "lily", "violet"],
    D={"rose": 27, "lily": 21, "violet": 9},
    P={"one": 130, "two": 100},
    Q={
        ("rose", "one"): 1.5,
        ("rose", "two"): 1,
        ("lily", "one"): 1,
        ("lily", "two"): 1,
        ("violet", "one"): 0.3,
        ("violet", "two"): 0.5,
    },
)
