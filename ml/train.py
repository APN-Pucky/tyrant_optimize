import tqdm
import yaml
from smpl_io.io import sed
from yaml.loader import SafeLoader
import argparse
import pandas
from numpy import asarray, asmatrix
from numpy import mean
from numpy import std
from sklearn.datasets import make_regression
from sklearn.model_selection import cross_val_score
from sklearn.model_selection import RepeatedStratifiedKFold
from xgboost.sklearn import XGBRegressor
from sklearn.model_selection import RepeatedKFold
from sklearn2pmml import sklearn2pmml
from sklearn2pmml.pipeline import PMMLPipeline

hash_cache = {}

def hash_to_ids_ext_b64(hash, nmin=12):
    if hash in hash_cache:
        ids = hash_cache[hash].copy()
    else:
        ids = []
        c = 0
        while c < len(hash):
            # for pc in hash:
            id = 0
            factor = 1
            p = base64_chars.find(hash[c])
            if p == -1:
                raise RuntimeError("Invalid hash character")
            d = p
            while d < 32:
                id += factor * d
                factor *= 32
                c = c + 1
                pc = hash[c]
                p = base64_chars.find(pc)
                if p == -1:
                    raise RuntimeError("Invalid hash character")
                d = p
            id += factor * (d - 32)
            c = c + 1
            ids.append(id)
        hash_cache[hash] = ids
        ids = ids.copy()
    while len(ids) < nmin:
        ids.append(0)
    return ids


parser = argparse.ArgumentParser(
    prog="Machine Learning for TUO",
    description="Train a model for TUO",
    epilog="Author: Alexander Puck Neuwirth",
)
parser.add_argument(
    "-d", "--database", help="database file", type=str, default="database.yml"
)
parser.add_argument("-o", "--output", help="output directory", type=str, default=".")
parser.add_argument(
    "-l",
    "--limit",
    help="minimum iterations needed to be included",
    type=int,
    default=10000,
)
args = parser.parse_args()

print("Loading database from " + args.database + "...", end="", flush=True)
base64_chars = "ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz0123456789+/"

db = yaml.load(sed("\t", "  ", args.database), Loader=SafeLoader)
# print(db.keys())
l = list(db.keys())[2:]
# print(db[l[0]])
print("done")


print("Parsing database...")
df = pandas.DataFrame()
xdata = []  # 10 + 10 ids each
wdata = []  # one score
sdata = []  # one score
ldata = []  # one score
pdata = []  # one score
dic = None
dfs = []
for ll in tqdm.tqdm(l, position=0, leave=True):
    hx = hash_to_ids_ext_b64(ll)
    # print(hx)
    for k in tqdm.tqdm(db[ll].keys(), position=1, leave=True):
        hk = hash_to_ids_ext_b64(k)
        for kk in db[ll][k]:
            res = db[ll][k][kk].split(" ")
            if int(res[-1]) >= args.limit:
                dic = {}
                for i, v in enumerate(hx):
                    dic["f" + str(i)] = v
                for i in range(0, len(hk)):
                    dic["y" + str(i)] = hk[i]
                hkk = hash_to_ids_ext_b64(kk)
                for i in range(0, len(hkk)):
                    dic["e" + str(i)] = hkk[i]
                dic["w"] = float(res[0]) / float(res[-1])
                dic["s"] = float(res[1]) / float(res[-1])
                dic["l"] = float(res[2]) / float(res[-1])
                dic["p"] = float(res[3]) / float(res[-1])
                dfs += [pandas.DataFrame.from_dict(dic,orient='index').T]
                #df = df.append(dic,ignore_index=True)
                # print(k,hash_to_ids_ext_b64(k), kk , hash_to_ids_ext_b64(kk), float(res[0]) / float(res[-1]), res[-1])

df = pandas.concat(dfs,ignore_index=True)
if dic is None:
    raise RuntimeError("No data found")
# print(dic)
# print(df.columns.difference(['w','s','l','p']))
# evaluate xgboost algorithm for classification


# X, y = make_regression(n_samples=1000, n_features=20, n_informative=15, noise=0.1, random_state=7)
# print(xdata)
# xdata = df[df.columns.difference(['w','s','l','p'])].to_numpy()
## define the model
# wmodel = XGBRegressor()
# wmodel.fit(xdata, wdata)
#
xdata = df[df.columns.difference(["w", "s", "l", "p"])]

# evaluate the model
# cv = RepeatedKFold(n_splits=10, n_repeats=3, random_state=1)
# n_scores = cross_val_score(model, X, y, scoring='neg_mean_absolute_error', cv=cv, n_jobs=-1, error_score='raise')
# report performance
# print('MAE: %.3f (%.3f)' % (mean(n_scores), std(n_scores)))

# aOhdOsUVtZJvZJvXSwPN9dW9cO+MN/ [1498, 12765, 14004, 15673, 15673, 16983, 30127, 30429, 31196, 32172, 0, 0] aOhdOsUVtZJvZJvXSwPN9dW9cO+ME/MN/ [1498, 12765, 14004, 15673, 15673, 16983, 30127, 30429, 31196, 31884, 32172, 0]
# print(wmodel.predict([[1498, 12765, 14004, 15673, 15673, 16983, 30127, 30429, 31196, 32172, 0, 0,1498, 12765, 14004, 15673, 15673, 16983, 30127, 30429, 31196, 31884, 32172, 0]]))
print("Training...")

print("WIN......", end="", flush=True)
wpmml_pipeline = PMMLPipeline([("regressor", XGBRegressor())])
wpmml_pipeline.fit(xdata, df["w"])
wpmml_pipeline.configure(compact=False)
sklearn2pmml(wpmml_pipeline, args.output + "/win.pmml")
cv = RepeatedKFold(n_splits=10, n_repeats=3, random_state=1)
scores = cross_val_score(
    wpmml_pipeline, xdata, df["w"], scoring="neg_mean_absolute_error", cv=cv, n_jobs=-1
)
print("Average Error: +-%.3f%% (+-%.3f%%)" % (-scores.mean()*100, scores.std()*100))

print("STALL....", end="", flush=True)
spmml_pipeline = PMMLPipeline([("regressor", XGBRegressor())])
spmml_pipeline.fit(xdata, df["s"])
spmml_pipeline.configure(compact=False)
sklearn2pmml(spmml_pipeline, args.output + "/stall.pmml")
cv = RepeatedKFold(n_splits=10, n_repeats=3, random_state=1)
scores = cross_val_score(
    spmml_pipeline, xdata, df["s"], scoring="neg_mean_absolute_error", cv=cv, n_jobs=-1
)
print("Average Error: +-%.3f%% (+-%.3f%%)" % (-scores.mean()*100, scores.std()*100))

print("LOSS.....", end="", flush=True)
lpmml_pipeline = PMMLPipeline([("regressor", XGBRegressor())])
lpmml_pipeline.fit(xdata, df["l"])
lpmml_pipeline.configure(compact=False)
sklearn2pmml(lpmml_pipeline, args.output + "/loss.pmml")
cv = RepeatedKFold(n_splits=10, n_repeats=3, random_state=1)
scores = cross_val_score(
    lpmml_pipeline, xdata, df["l"], scoring="neg_mean_absolute_error", cv=cv, n_jobs=-1
)
print("Average Error: +-%.3f%% (+-%.3f%%)" % (-scores.mean()*100, scores.std()*100))

print("POINTS...", end="", flush=True)
ppmml_pipeline = PMMLPipeline([("regressor", XGBRegressor())])
ppmml_pipeline.fit(xdata, df["p"])
ppmml_pipeline.configure(compact=False)
sklearn2pmml(ppmml_pipeline, args.output + "/points.pmml")
cv = RepeatedKFold(n_splits=10, n_repeats=3, random_state=1)
scores = cross_val_score(
    ppmml_pipeline, xdata, df["p"], scoring="neg_mean_absolute_error", cv=cv, n_jobs=-1
)
print("Average Error: +-%.3f (+-%.3f)" % (-scores.mean(), scores.std()))


# print(wpmml_pipeline.predict([[1498, 12765, 14004, 15673, 15673, 16983, 30127, 30429, 31196, 32172, 0, 0,1498, 12765, 14004, 15673, 15673, 16983, 30127, 30429, 31196, 31884, 32172, 0]]))
# print(spmml_pipeline.predict([[1498, 12765, 14004, 15673, 15673, 16983, 30127, 30429, 31196, 32172, 0, 0,1498, 12765, 14004, 15673, 15673, 16983, 30127, 30429, 31196, 31884, 32172, 0]]))
# print(lpmml_pipeline.predict([[1498, 12765, 14004, 15673, 15673, 16983, 30127, 30429, 31196, 32172, 0, 0,1498, 12765, 14004, 15673, 15673, 16983, 30127, 30429, 31196, 31884, 32172, 0]]))
# print(ppmml_pipeline.predict([[1498, 12765, 14004, 15673, 15673, 16983, 30127, 30429, 31196, 32172, 0, 0,1498, 12765, 14004, 15673, 15673, 16983, 30127, 30429, 31196, 31884, 32172, 0]]))
# 	const char* pc = hash;
# 	while (*pc)
# 	{
# 		unsigned id = 0;
# 		unsigned factor = 1;
# 		const char* p = strchr(base64_chars, *pc);
# 		if (!p)
# 		{ throw std::runtime_error("Invalid hash character"); }
# 		size_t d = p - base64_chars;
# 		while (d < 32)
# 		{
# 			id += factor * d;
# 			factor *= 32;
# 			++ pc;
# 			p = strchr(base64_chars, *pc);
# 			if (!p)
# 			{ throw std::runtime_error("Invalid hash character"); }
# 			d = p - base64_chars;
# 		}
# 		id += factor * (d - 32);
# 		++ pc;
# 		ids.push_back(id);
# 	}
