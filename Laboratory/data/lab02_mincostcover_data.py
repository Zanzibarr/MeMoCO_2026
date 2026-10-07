from dataclasses import dataclass


@dataclass(frozen=True)
class MinCostCoverData:
    model_name: str
    var_type: str  # type of I variables
    I: list[str]  # resources
    J: list[str]  # requests
    C: dict[str, float]  # unit cost of resource i in I
    R: dict[str, float]  # requested amount of j in J
    A: dict[
        tuple[str, str], float
    ]  # amount of request j in J satisfied by one unit of resource i in I


diet = MinCostCoverData(
    model_name="prodmix_farmer",
    var_type="cont",
    I=["V", "M", "F"],
    J=["proteins", "iron", "calcium"],
    C={"V": 4, "M": 10, "F": 7},
    R={"proteins": 20, "iron": 30, "calcium": 10},
    A={
        ("V", "proteins"): 5,
        ("V", "iron"): 6,
        ("V", "calcium"): 5,
        ("M", "proteins"): 15,
        ("M", "iron"): 10,
        ("M", "calcium"): 3,
        ("F", "proteins"): 4,
        ("F", "iron"): 5,
        ("F", "calcium"): 12,
    },
)
