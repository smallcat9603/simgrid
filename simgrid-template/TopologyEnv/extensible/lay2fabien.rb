#!/usr/bin/ruby -Ku
# 
# 	ラック配置→トポロジ読込／生成
# 	Layout racks on a floor and then create/load per-node topologies
# 	Ikki Fujiwara <ikki@nii.ac.jp>
# 

RACK_WIDTH     =  60 # Rack width [cm]
RACK_DEPTH     = 210 # Rack depth including aisle [cm]
OVERHEAD_OUTER = 400 # Inter-rack cabling overhead [cm]
OVERHEAD_INNER = 200 # Intra-rack cabling overhead [cm]
RAND_TRIAL     =  10 # Number of trial of random matching

class CostModel
	# from Mudigonda's "Taming the flying cable monster" paper (2011)
	#   Switch cost
	#     $500/port
	#   Custom cable cost
	#     Single-channel (server <-> switch)
	#       copper $6/m + $40  (<=5m)
	#     Quad-channel (switch <-> switch)
	#       copper $16/m + $40 (<=5m)
	#       fiber  $5/m + $376 (>5m)
	#     plus 25% as a manufacturing cost
	#   Stock cable cost
	#     SFP copper (server <-> switch)
	#       1m $45; 2m $52; 3m $66; 5m $74; 10m $101; 12m $117
	#     QSFP copper (switch <-> switch, <=5m)
	#       1m $55; 2m $74; 3m $87; 5m  $116
	#     QSFP+ fiber (switch <-> switch, >5m)
	#       10m $418; 15m $448; 20m $465; 30m $508; 50m $618; 100m $883
	#   Deployment cost
	#     intra-rack $2.50
	#     inter-rack $6.25
	def cable(length)
		if length <= $innerhead
			dollar = (length.to_f / 100 * 16 + 40) * 1.25 + 2.50
		elsif length <= 500
			dollar = (length.to_f / 100 * 16 + 40) * 1.25 + 6.25
		else
			dollar = (length.to_f / 100 * 5 + 376) * 1.25 + 6.25
		end
		(dollar * 100).round.to_f / 100
	end
	def switch(nports)
		dollar = nports * 500
		dollar
	end
end

$help_message = <<-EOM
Usage:
  #{$0} [Options] Layout Input_file|Topologies...

Options:
  -f	overwrite existing log file
  -c	create per-rack (clustered) edgelist file
  --overhead <OUTER>,<INNER>
    	inter-rack and intra-rack additional cable length [cm]
    	default: #{OVERHEAD_OUTER},#{OVERHEAD_INNER}
  --layout <SHAPE>
    	<X>x<Y> => <X> racks along depth, <Y> racks along width
    	room => square-room layout (e.g. 2 x 8 racks)
    	mesh => square-number layout (e.g. 4 x 4 racks)
    	default: mesh
  --seed <SEED>
    	random seed

Layout:
  <N>r<Z>	generate <N>-node layout with <Z>-sized racks

Input_file and/or Topologies:
  <FILE>	per-node (non-clustered) edgelist
  hc    	hypercube, auto-sized
  hc-<D>	hypercube
  mesh  	3-D mesh, auto-sized
  mesh-<C>	<C> mesh
  torus 	3-D torus, auto-sized
  torus-<C>	<C> torus
  ring  	1-D torus, auto-sized
  rr-<D>	random ring
  r-<D> 	random
  hx    	hyperx
  df    	dragonfly, fully-connected
  sky   	skywalk
  fb1-<D>[-<B>]@<FILE>	randomly picked from per-node edgelist
  fb2-<D>[-<B>]@<FILE>	intra-rack: fully-connected, inter-rack: randomly picked from per-rack edgelist
where
  <C>   	#nodes for each dimension (ex. '5-4-3')
  <D>   	degree (ex. '6')
  <B>   	failure rate of edgelist (ex. '0.25')
  <FILE>	edgelist filename (ex. 'mygraph.edges')
See 'wire' method in #{$0} for complete list of available topologies.

Standard output:
See 'disp_cost' method in #{$0} for details of the values displayed.
EOM

require "optparse"
require "pp"

def main
	$opts = ARGV.getopts("fqc", "overhead:#{OVERHEAD_OUTER},#{OVERHEAD_INNER}", "layout:mesh", "seed:")
	if ARGV.length < 2
		puts $help_message
		exit(1)
	end
	dimension = ARGV[0]
	parameter = ARGV.collect{|x| File.basename(x, ".*") }.join("+")
	parameter += "[#{$opts['seed'].to_i}]" if $opts["seed"]
	mapfn = "#{dimension}.coord"
	outfn = "#{parameter}.edges"
	clufn = "#{parameter}.clu.edges"
	if File.exist?(outfn) && !$opts["f"]
		STDERR.puts "Output file '#{outfn}' exists. Use -f to overwrite" unless $opts["q"]
		exit(0)
	end
	$outerhead = $opts["overhead"].scan(/\d+/)[0].to_i
	$innerhead = $opts["overhead"].scan(/\d+/)[1].to_i
	if $opts["layout"] =~ /\d+x\d+/
		$layout_scheme = $opts["layout"].scan(/\d+/)
	elsif $opts["layout"] == "room"
		$layout_scheme = :room
	else
		$layout_scheme = :mesh
	end
	srand($opts["seed"].to_i)
	
	g = MyNetwork.new
	# ノードリストを生成
	dimensions = dimension.scan(/\d+/).collect{|x| x.to_i }
	g.make_layout(dimensions)
	# リンクを張る or エッジリストを読み込む
	ARGV[1..-1].each do |arg|
		if File.exist?(arg)
			g.load_edgelist(arg)
		else
			g.wire(arg)
		end
	end
	# エッジリストとマップを書き出す
	g.save_edgelist(outfn)
	g.save_layout(mapfn)
	# ラック単位のエッジリストを書き出す
	if $opts["c"]
		g.save_clustered_edgelist(clufn)
	end
	# コストを表示
	g.disp_cost(parameter)
end

# 配列
class Array
	# 和
	def sum
		self.inject(0){|x, y| x + y }
	end
	# 積
	def mul
		self.inject(1){|x, y| x * y }
	end
	# 平均
	def avg
		self.sum.to_f / self.length
	end
	# 要素をランダムに取り除く
	def failure(rate)
		(rate * self.length).to_i.times do
			self.delete_at(rand(self.length))
		end
		self
	end
	# 複数の配列の同じ位置にある要素同士を組み合わせてイテレート
	# 	[[1,2,3,4],[5,6,7],[8,9]].each_pair => [1,5,8],[2,6,9],[3,7,nil],[4,nil,nil]
	def each_pair
		self.max_by{|x| x.length }.each_index do |i|
			yield(self.collect{|x| x[i] })
		end
	end
	# 複数の配列からそれぞれ1個の要素を選んだ配列の配列を返す
	# 	[[1,2],[3,4,5]].self_product => [[1,3],[1,4],[1,5],[2,3],[2,4],[2,5]]
	def self_product
		if self.empty?
			[[]]
		elsif self[0].is_a?(Array)
			self[0].product(*self[1..-1])
		else
			raise TypeError
		end
	end
	# ランダムマッチングを生成
	# 	ブロックにペア案が渡される。true を返せば採用、false を返せば不採用。
	def random_matching
		pairs = Array.new
		pool = self.shuffle
		until pool.empty?
			x = pool.shift
			pool.each_with_index do |y, i|
				if yield(x, y)
					pool.delete_at(i)
					pairs.push([x, y])
					break
				end
			end
		end
		pairs
	end
end

# 行列
# 行番号・列番号は0オリジン
class MyMatrix
	include Enumerable
	attr_reader :row_size, :col_size
	def initialize
		@data = Hash.new{|hash, key|
			hash[key] = Hash.new(0) # デフォルト値
		}
		@row_size = 0
		@col_size = 0
	end
	def initialize_copy(org)
		@data = Hash.new{|hash, key|
			hash[key] = Hash.new(0) # デフォルト値
		}
		org.each do |i, j, val|
			@data[i][j] = val
		end
		@row_size = org.row_size
		@col_size = org.col_size
	end
	def [](i, j)
		@data[i][j]
	end
	def []=(i, j, val)
# 		raise TypeError unless i.is_a?(Integer) && j.is_a?(Integer) && val.is_a?(Numeric) # 重い
		@data[i][j] = val
		@row_size = i + 1 if row_size < i + 1
		@col_size = j + 1 if col_size < j + 1
	end
	# スカラー和
	def +(other)
# 		raise TypeError unless other.is_a?(Numeric) # 重い
		a = self.class.new
		@row_size.times do |i|
			@col_size.times do |j|
				a[i, j] = @data[i][j] + other
			end
		end
		a
	end
	# スカラー積
	def *(other)
# 		raise TypeError unless other.is_a?(Numeric) # 重い
		a = self.class.new
		@data.each do |i, r|
			r.each do |j, val|
				a[i, j] = val * other
			end
		end
		a
	end
	def empty?
		@data.empty?
	end
	def each
		@data.each do |i, r|
			r.each do |j, val|
				yield(i, j, val)
			end
		end
	end
	def each_sorted
		@data.keys.sort.each do |i|
			@data[i].keys.sort.each do |j|
				yield(i, j, @data[i][j])
			end
		end
	end
	def along_row(i)
		@data[i].each do |j, val|
			yield(j, val)
		end
	end
	def along_col(j)
		@data.each do |i, r|
			yield(i, r[j])
		end
	end
	def to_s
		a = Array.new
		@row_size.times do |i|
			b = Array.new
			@col_size.times do |j|
				b.push(@data[i][j].to_s)
			end
			a.push(b.join(" "))
		end
		a.join("\n")
	end
	# エッジリストを隣接行列として読み込む
	def load_edgelist(filename, undirected = true)
		IO.foreach(filename) do |line|
			i, j, val = line.split # split by whitespaces
			next unless i && j
			i = i.to_i
			j = j.to_i
			val = (val) ? val.to_i : 1
			self[i, j] = val
			self[j, i] = val if undirected
		end
		self
	end
	# 重み付きエッジリストとして書き出す
	def save_edgelist(filename)
		open(filename, "w") do |f|
			each_sorted do |i, j, val|
				f.printf("%d %d %d\n", i, j, val)
			end
		end
	end
end

# 無向グラフ
class MyGraph
	require "set"
	include Enumerable
	attr_reader :edgeset
	def initialize
		@edgeset = Set.new
		@adjmat = MyMatrix.new
	end
	def initialize_copy(org)
		@edgeset = org.edgeset.dup # 注意! SortedSet の .dup はシャローコピーだお
		@adjmat = org.adj_matrix.dup
	end
	def edges
		@edgeset.to_a
	end
	def edge_count
		@edgeset.size
	end
	def adj_matrix
		@adjmat
	end
	def each_edge
		@edgeset.each do |e|
			yield(e)
		end
	end
	alias :each :each_edge
	def each_edge_sorted
		@edgeset.sort.each do |e|
			yield(e)
		end
	end
	def has_edge?(i, j)
		e = (i < j) ? [i, j] : [j, i]
		@edgeset.include?(e)
	end
	def add_edge(i, j)
		e = (i < j) ? [i, j] : [j, i]
		@edgeset.add(e)
		@adjmat[e[0], e[1]] = @adjmat[e[1], e[0]] = 1 # 対称行列、重みなし
		self
	end
	def add_edges(arr)
		arr.each do |i, j|
			add_edge(i, j)
		end
		self
	end
	def delete_edge(i, j)
		e = (i < j) ? [i, j] : [j, i]
		@edgeset.delete(e)
		@adjmat[e[0], e[1]] = @adjmat[e[1], e[0]] = 0 # 対称行列、重みなし
		self
	end
	def delete_edges(arr)
		arr.each do |i, j|
			delete_edge(i, j)
		end
		self
	end
	# 隣接頂点リスト
	def adj_list(i)
		adjset = SortedSet.new
		@adjmat.along_row(i) do |j, w|
			adjset.add(j) if w != 0
		end
		adjset.to_a
	end
	# 頂点次数
	def degree(i)
		d = 0
		@adjmat.along_row(i) do |j, w|
			d += 1 if w != 0
		end
		d
	end
	# エッジリストを読み込む
	def load_edgelist(filename)
		open(filename, "r") do |f|
			f.each_line do |line|
				i, j, w = line.split # split by whitespaces
				next unless i && j
				i = i.to_i
				j = j.to_i
				add_edge(i, j)
			end
		end
	end
	# エッジリストを書き出す
	def save_edgelist(filename)
		open(filename, "w") do |f|
			each_edge_sorted do |i, j|
				f.printf("%d %d\n", i, j)
			end
		end
	end
end

# 重み付き無向グラフ
class MyWeightedGraph < MyGraph
	# 重みの数だけ重複させたエッジの配列
	def multiple_edges
		a = Array.new
		@edgeset.sort.each do |e|
			@adjmat[e[0], e[1]].times do
				a.push(e)
			end
		end
		a
	end
	def add_edge(i, j, w = 1)
		e = (i < j) ? [i, j] : [j, i]
		@edgeset.add(e)
		@adjmat[e[0], e[1]] += w # 対称行列
		@adjmat[e[1], e[0]] += w # 対称行列
		self
	end
	def add_edges(arr)
		arr.each do |i, j|
			add_edge(i, j)
		end
		self
	end
	def delete_edge(i, j, w = 1)
		e = (i < j) ? [i, j] : [j, i]
		@adjmat[e[0], e[1]] -= w if @adjmat[e[0], e[1]] >= w # 対称行列
		@adjmat[e[1], e[0]] -= w if @adjmat[e[1], e[0]] >= w # 対称行列
		@edgeset.delete(e) if @adjmat[e[0], e[1]] == 0 || @adjmat[e[1], e[0]]
		self
	end
	def delete_edges(arr)
		arr.each do |i, j|
			delete_edge(i, j)
		end
		self
	end
	# エッジリストを読み込む
	def load_edgelist(filename)
		open(filename, "r") do |f|
			f.each_line do |line|
				i, j, w = line.split # split by whitespaces
				next unless i && j
				i = i.to_i
				j = j.to_i
				w = w.to_i
				add_edge(i, j, w)
			end
		end
	end
	# エッジリストを書き出す
	def save_edgelist(filename)
		open(filename, "w") do |f|
			each_edge_sorted do |i, j|
				w = @adjmat[i, j]
				f.printf("%d %d %d\n", i, j, w)
			end
		end
	end
end

# 座標つき無向グラフ
# 	ラックの奥行方向が X 軸、幅方向が Y 軸、ラック内が Z 軸。(alloc2.rb は X, Y が逆)
class MyMap < MyGraph
	def initialize
		super
		@nodes = Array.new # 全ノードリスト
		@racks = Array.new # 全ラックリスト
		@racksize = 0 # 1ラックあたり最大ノード数
		@xs = Array.new # X 座標リスト (ラックの奥行方向)
		@ys = Array.new # Y 座標リスト (ラックの幅方向)
		@nodes_in = Hash.new # @nodes_in[ラック番号] = ノードリスト
		@rack_of  = Hash.new # @rack_of[ノード番号] = ラック番号
		@coord_of = Hash.new # @coord_of[ラック番号] = 座標
		@rack_at  = Hash.new # @rack_at[座標] = ラック番号
		@dist_max = 0 # 対角ラック間距離 (cm)
		@cable_total = 0 # 総配線長 (cm)
	end
	# レイアウトを生成
	def make_layout(params)
# 		STDERR.puts "<<<<<<<< make_layout (#{params}) >>>>>>>>"
		raise "Specify the number of nodes" unless params[0]
		raise "Specify the rack size" unless params[1]
		nnode = params[0].to_i
		@racksize = params[1].to_i
		nrack = (nnode.to_f / @racksize).ceil # ラック数
		if $layout_scheme.is_a?(Array)
			nx = $layout_scheme[0].to_i
			ny = $layout_scheme[1].to_i
			raise "Lack of space for racks" unless nrack <= nx * ny
		elsif $layout_scheme == :room
			ra = Math.sqrt(nrack * RACK_WIDTH * RACK_DEPTH) # 部屋の辺長
			nx = [(ra / RACK_DEPTH).floor, 1].max # 奥行方向の設置数
			ny = (nrack.to_f / nx).ceil # 幅方向の設置数
		else
			nx = Math.sqrt(nrack).ceil  # 奥行方向の設置数
			ny = (nrack.to_f / nx).ceil # 幅方向の設置数
		end
		@nodes = (0...nnode).to_a
		@racks = (0...nrack).to_a
		@nodes.each do |n|
			r = ((n * nrack).to_f / nnode).floor
			@nodes_in[r] ||= Array.new
			@nodes_in[r].push(n)
			@rack_of[n] = r
		end
		@racks.each do |r|
			x = (r / ny) * RACK_DEPTH
			y = (r % ny) * RACK_WIDTH
			@coord_of[r] = [x, y]
			@rack_at[[x, y]] = r
			@xs.push(x)
			@ys.push(y)
		end
		@xs = @xs.uniq.sort
		@ys = @ys.uniq.sort
		calc_distances
	end
	# レイアウトを読み込む
	def load_layout(filename)
		data = Hash.new{|hash, key|
			hash[key] = Set.new
		}
		open(filename, "r") do |f|
			f.each_line do |line|
				# *.coord ファイルは X, Y が逆
				n, r, y, x = line.split # split by whitespaces
				next unless n && r && y && x
				n = n.to_i # ノード番号
				r = r.to_i # ラック番号
				x = x.to_i # X 座標
				y = y.to_i # Y 座標
				data[r].add(n)
				@rack_of[n] = r
				@coord_of[r] = [x, y]
				@rack_at[[x, y]] = r
				@xs.push(x)
				@ys.push(y)
			end
		end
		data.each do |r, nset|
			@nodes_in[r] = nset.sort
			@nodes.concat(@nodes_in[r])
			@racksize = nset.length if @racksize < nset.length
		end
		@nodes.sort!
		@racks = data.keys.sort
		@xs = @xs.uniq.sort
		@ys = @ys.uniq.sort
		calc_distances
	end
	# レイアウトを書き出す
	def save_layout(filename)
		open(filename, "w") do |f|
			@rack_of.each do |node, rack|
				x, y = @coord_of[rack]
				# *.coord ファイルは X, Y が逆
				f.printf("%d %d %d %d\n", node, rack, y, x)
			end
		end
	end
	# 配線長を重みとするエッジリストを書き出す
	def save_edgelist(filename)
		open(filename, "w") do |f|
			each_edge_sorted do |n1, n2|
				f.printf("%d %d %d\n", n1, n2, cable_between(n1, n2))
			end
		end
	end
	# 配線数を重みとするラック単位のエッジリストを書き出す
	def save_clustered_edgelist(filename)
		rckmat = MyMatrix.new
		each_edge do |n1, n2|
			r1 = @rack_of[n1]
			r2 = @rack_of[n2]
# 			rckmat[r1, r2] += 1 # 同一ラック内も含める
			rckmat[r1, r2] += 1 if r1 != r2 # 同一ラック内を含めない
		end
		rckmat.save_edgelist(filename)
# 		return
		# ログファイルも作る
		ntube = 0
		nlink = 0
		rckmat.each do |i, j, w|
			raise "oops" unless i < j
			ntube += 1
			nlink += w
		end
		logfn = filename.sub(/\.\w+$/){ ".log" }
		open(logfn, "w") do |f|
			f.printf("#node      = %d\n", @nodes.length)
			f.printf("#edge      = %d\n", edge_count)
			f.printf("#rack      = %d\n", @racks.length)
			f.printf("#tube      = %d\n", ntube)
			f.printf("#link      = %d\n", nlink)
			f.printf("membership = %s\n", @nodes.collect{|n| @rack_of[n] }.join(","))
		end
	end
	# コストを表示
	def disp_cost(parameter)
		cost_of = CostModel.new
		cable_count = 0
		cable_amount = 0
		cable_cost = 0
		switch_cost = 0
		each_edge do |n1, n2|
			cable_count += 1
			cable_amount += cable_between(n1, n2)
			cable_cost += cost_of.cable(cable_between(n1, n2))
# 			puts "  #{n1}--#{n2}\t#{cable_between(n1, n2)}cm\t$#{cost_of.cable(cable_between(n1, n2))}"
		end
		@nodes.each do |n|
# 			puts "  #{n}\tdeg=#{degree(n)}\t$#{cost_of.switch(degree(n))}"
			switch_cost += cost_of.switch(degree(n))
		end
		total_cost = cable_cost + switch_cost
		STDOUT.puts format("%s\t%0.2f\t%0.2f\t%0.2f\t%0.2f\t%d\t%d\t%0.2f", parameter, total_cost, cable_cost, switch_cost, 0.0, cable_amount, cable_count, cable_amount.to_f / cable_count)
	end
	# 距離計算
	def calc_distances
		@racks.each do |r|
			x = dist_between(@coord_of[0], @coord_of[r])
			@dist_max = x if @dist_max < x
		end
	end
	# 配線長
	def cable_between(n1, n2)
		cable_of(dist_manhattan(@coord_of[@rack_of[n1]], @coord_of[@rack_of[n2]]))
	end
	# 距離 → 配線長
	def cable_of(dist)
		cable = (dist == 0) ? $innerhead : $outerhead + dist
		cable.round
	end
	# ユークリッド距離
	def dist_euclidean(p, q)
		Math.sqrt((p[0] - q[0]) ** 2 + (p[1] - q[1]) ** 2)
	end
	# マンハッタン距離
	def dist_manhattan(p, q)
		(p[0] - q[0]).abs + (p[1] - q[1]).abs
	end
	alias :dist_between :dist_manhattan
	# リンク追加
	def add_link(n1, n2)
		return if !n1 || !n2 || n1 == n2
# 		STDERR.puts "  #{n1}--#{n2}"
		n = edge_count
		add_edge(n1, n2)
		@cable_total += cable_between(n1, n2) if edge_count > n
	end
	def add_links(arr)
		arr.each do |n1, n2|
			add_link(n1, n2)
		end
	end
	# リンク削除
	def delete_link(n1, n2)
		n = edge_count
		delete_edge(n1, n2)
		@cable_total -= cable_between(n1, n2) if edge_count < n
	end
	def delete_links(arr)
		arr.each do |n1, n2|
			delete_link(n1, n2)
		end
	end
	# ラックを X 方向に串刺し
	def racks_along_x
		@ys.each do |y| 
			yield(@xs.collect{|x| @rack_at[[x, y]] }.compact)
		end
	end
	# ラックを Y 方向に串刺し
	def racks_along_y
		@xs.each do |x|
			yield(@ys.collect{|y| @rack_at[[x, y]] }.compact)
		end
	end
	# ノードを X 方向に串刺し
	def nodes_along_x
		racks_along_x do |rs|
			@racksize.times do |z|
				yield(rs.collect{|r| @nodes_in[r][z] }.compact)
			end
		end
	end
	# ノードを Y 方向に串刺し
	def nodes_along_y
		racks_along_y do |rs|
			@racksize.times do |z|
				yield(rs.collect{|r| @nodes_in[r][z] }.compact)
			end
		end
	end
	# ノードを Z 方向に串刺し
	def nodes_along_z
		@racks.each do |r|
			yield(@nodes_in[r])
		end
	end
	# 各ラック内の各ノードをサイクリックに取得する Enumerator
	# 	enum[ラック番号].next = 次に使うノード番号
	def enum_nodes_cyclic
		enum = Hash.new 
		@racks.each do |r|
			enum[r] = @nodes_in[r].cycle
		end
		enum
	end
	# ラックを頂点とするグラフ
	def graph_of_racks
		g = MyGraph.new
		self.edges.each do |n1, n2|
			g.add_edge(@rack_of[n1], @rack_of[n2])
		end
		g
	end
	# ラックを頂点とする重み付きグラフ
	def weighted_graph_of_racks
		g = MyWeightedGraph.new
		self.edges.each do |n1, n2|
			g.add_edge(@rack_of[n1], @rack_of[n2])
		end
		g
	end
end

# ネットワーク
class MyNetwork < MyMap
	def initialize
		super
	end
	# リンクを張る
	def wire(arg)
		if /^(.+?)@(.+)$/ =~ arg
			arg1 = $1
			filename = $2
			raise "Input file #{filename} does not exist" unless File.exist?(filename)
		else
			arg1 = arg
			filename = nil
		end
		opts = arg1.split(/-/)
		topo = opts.shift
		prms = opts.collect{|x| x.to_i }
		fullmesh = [@xs.length, @ys.length]
		fullcube = [@xs.length, @ys.length, @racksize]
		unit = {
			"km" => 100000,
			"m"  => 100,
			"cm" => 1
		}
		begin
			case topo
				# <F>	edgelist filename
				# <D>	degree
				# <C>	#nodes for each dimension (ex. '5-4-3')
				# <E>	degrees for each dimension (ex. '3-2-1')
				# <L>	cable length (ex. '900m', '30km')
				# <R>	radius of neighborhood (% of diagonal distance)
			when "hc"
				# hc    	hypercube, auto-sized
				# hc-<D>	hypercube
				prms[0] ||= (Math.log(@nodes.length) / Math.log(2)).ceil
				linecube([2] * prms[0])
			when "mesh"
				# mesh  	3-D mesh, auto-sized
				# mesh-<C>	<C>-D mesh
				prms = fullcube if prms.empty?
				linecube(prms)
			when "torus"
				# torus 	3-D torus, auto-sized
				# torus-<C>	<C>-D torus
				prms = fullcube if prms.empty?
				ringcube(prms)
			when "ring"
				# ring  	1-D torus, auto-sized
				ringcube([@nodes.length])
			when "rr"
				# rr-<D>	random ring
				ringcube([@nodes.length])
				random(prms[0] - 2)
			when /rr~(\d+)([kc]?m)/
				# rr~<L>	random ring, limited cable length
				ringcube([@nodes.length])
				random_upto($1.to_i * unit[$2])
			when "sky"
				# sky   	skywalk
				skywalk(opts, [])
			when /sky~(\d+)([kc]?m)/
				# sky~<L>	skywalk, limited cable length
				skywalk(opts, [$1.to_i * unit[$2]])
			when "hx"
				# hx    	hyperx
				hyperx(opts)
			when "df"
				# df    	dragonfly, fully-connected
				dragonfly(opts)
			when "di"
				# di-<D>	partial dragonfly, intra-rack random
				dragonfly_random_inside(prms[0])
			when "do"
				# do-<D>	partial dragonfly, inter-rack random
				# do-<D>-<R>	partial dragonfly, inter-rack random neighbor
				dragonfly_random_outside(prms[0], prms[1])
			when "ds"
				# ds-<D>	partial dragonfly, inter-rack random straight
				# ds-<D>-<R>	partial dragonfly, inter-rack random straight neighbor
				dragonfly_random_outside_straight(prms[0], prms[1])
			when "da"
				# da-<D>	partial dragonfly, inter-rack random diagonal
				# da-<D>-<R>	partial dragonfly, inter-rack random diagonal neighbor
				dragonfly_random_outside_diagonal(prms[0], prms[1])
			when "dh"
				# dh-<D>	partial dragonfly, inter-rack hypercube
				# dh    	partial dragonfly, inter-rack hypercube, auto-sized
				prms[0] ||= (Math.log(@racks.length) / Math.log(2)).ceil
				dragonfly_linecube_outside([2] * prms[0])
			when "dm"
				# dm-<C>	partial dragonfly, inter-rack mesh
				# dm    	partial dragonfly, inter-rack 2-D mesh, auto-sized
				prms = fullmesh if prms.empty?
				dragonfly_linecube_outside(prms)
			when "dt"
				# dt-<C>	partial dragonfly, inter-rack torus
				# dt    	partial dragonfly, inter-rack 2-D torus, auto-sized
				prms = fullmesh if prms.empty?
				dragonfly_ringcube_outside(prms)
			when "rs"
				# rs-<E>	random straight
				straight(prms, :random)
			when "r"
				# r-<D> 	random
				random(prms[0])
			when "ri"
				# ri-<D>	intra-rack random
				random_inside(prms[0])
			when "ro"	
				# ro-<D>	inter-rack random
				random_outside(prms[0])
			when "fb1"
				# fb1-<D>[-<B>]@<F> 	randomly picked from per-node edgelist
				fabien1(prms[0], opts[1].to_f, filename)
			when "fb2"	
				# fb2-<D>[-<B>]@<F>	intra-rack: fully-connected, inter-rack: randomly picked from per-rack edgelist
				fabien2(prms[0], opts[1].to_f, filename)
			else
				raise "Topology '#{topo}' is not defined"
			end
		rescue RuntimeError
			STDERR.puts "wire('#{arg}'): #{$!}"
# 			STDERR.puts $@.collect{|s| "\tfrom #{s}" } # stack trace
			exit(1)
		end
# 		STDERR.print "'#{arg}' => cable #{@cable_total.to_f / 100}m\n"
	end
	# HyperX
	# 	Z方向・Y方向・X方向ノード間完全結合
	def hyperx(opts)
		raise "No parameter required" unless opts.length == 0
# 		STDERR.puts "<<<<<<<< HYPERX >>>>>>>>"
		nodes_along_z do |nz|
			link_full(nz)
		end
		nodes_along_y do |ny|
			link_full(ny)
		end
		nodes_along_x do |nx|
			link_full(nx)
		end
	end
	# ドラゴンフライ
	# 	Z方向ノード間完全結合 + 全ラック間完全結合
	def dragonfly(opts)
		raise "No parameter required" unless opts.length == 0
# 		STDERR.puts "<<<<<<<< DRAGONFLY >>>>>>>>"
		nodes_along_z do |nz|
			link_full(nz)
		end
		enum = enum_nodes_cyclic
		link_quasi_full(@racks, enum)
	end
	# ドラゴンフライの一部、ラック内ランダムリンク
	def dragonfly_random_inside(deg)
# 		STDERR.puts "<<<<<<<< DRAGONFLY_RANDOM_INSIDE (#{deg}) >>>>>>>>"
		maxdeg = @racksize - 1
		if deg < maxdeg
			@racks.each do |r|
				link_random(@nodes_in[r], deg)
			end
		else
			@racks.each do |r|
				link_full(@nodes_in[r])
			end
		end
	end
	# ドラゴンフライの一部、ラック間ランダムリンク
	def dragonfly_random_outside(deg, radius)
		raise "Invalid radius" unless radius == nil or radius > 0
# 		STDERR.puts "<<<<<<<< DRAGONFLY_RANDOM_OUTSIDE (#{deg}, #{radius}) >>>>>>>>"
		maxdist = (@dist_max * radius.to_f / 100).floor # % → cm
		maxdeg = ((@xs.length * @ys.length - 1).to_f / @racksize).ceil
		enum = enum_nodes_cyclic
		if deg < maxdeg
			link_quasi_random(@racks, deg, enum) do |r1, r2|
				!radius || dist_between(@coord_of[r1], @coord_of[r2]) <= maxdist
			end
		else
			link_quasi_full(@racks, enum) do |r1, r2|
				!radius || dist_between(@coord_of[r1], @coord_of[r2]) <= maxdist
			end
		end
	end
	# ドラゴンフライの一部、ラック間ランダム直進リンク
	def dragonfly_random_outside_straight(deg, radius)
		raise "Invalid radius" unless radius == nil or radius > 0
# 		STDERR.puts "<<<<<<<< DRAGONFLY_RANDOM_OUTSIDE_STRAIGHT (#{deg}, #{radius}) >>>>>>>>"
		maxdist = (@dist_max * radius.to_f / 100).floor # % → cm
		maxdeg = ((@xs.length - 1 + @ys.length - 1).to_f / @racksize).ceil
		enum = enum_nodes_cyclic
		if deg < maxdeg
			link_quasi_random(@racks, deg, enum) do |r1, r2|
				x1, y1 = @coord_of[r1]
				x2, y2 = @coord_of[r2]
				if x1 != x2 && y1 != y2
					false
				else
					!radius || dist_between(@coord_of[r1], @coord_of[r2]) <= maxdist
				end
			end
		else
			racks_along_y do |ry|
				link_quasi_full(ry, enum) do |r1, r2|
					!radius || dist_between(@coord_of[r1], @coord_of[r2]) <= maxdist
				end
			end
			racks_along_x do |rx|
				link_quasi_full(rx, enum) do |r1, r2|
					!radius || dist_between(@coord_of[r1], @coord_of[r2]) <= maxdist
				end
			end
		end
	end
	# ドラゴンフライの一部、ラック間ランダム非直進リンク
	def dragonfly_random_outside_diagonal(deg, radius)
		raise "Invalid radius" unless radius == nil or radius > 0
# 		STDERR.puts "<<<<<<<< DRAGONFLY_RANDOM_OUTSIDE_DIAGONAL (#{deg}, #{radius}) >>>>>>>>"
		maxdist = (@dist_max * radius.to_f / 100).floor # % → cm
		maxdeg = (((@xs.length - 1) * (@ys.length - 1)).to_f / @racksize).ceil
		enum = enum_nodes_cyclic
		link_quasi_random(@racks, deg, enum) do |r1, r2|
			x1, y1 = @coord_of[r1]
			x2, y2 = @coord_of[r2]
			if x1 == x2 || y1 == y2
				false
			else
				!radius || dist_between(@coord_of[r1], @coord_of[r2]) <= maxdist
			end
		end
	end
	# ドラゴンフライの一部、ラック間線状キューブ
	def dragonfly_linecube_outside(sizes)
# 		STDERR.puts "<<<<<<<< DRAGONFLY_LINECUBE_OUTSIDE (#{sizes}) >>>>>>>>"
		enum = enum_nodes_cyclic
		@cube_index = -1
		link_quasi_cube(@racks, sizes, false, enum)
	end
	# ドラゴンフライの一部、ラック間環状キューブ
	def dragonfly_ringcube_outside(sizes)
# 		STDERR.puts "<<<<<<<< DRAGONFLY_RINGCUBE_OUTSIDE (#{sizes}) >>>>>>>>"
		enum = enum_nodes_cyclic
		@cube_index = -1
		link_quasi_cube(@racks, sizes, true, enum)
	end
	# スカイウォーク
	# 	Z方向ノード間完全結合 + X方向・Y方向ラック間完全結合
	def skywalk(opts, lens)
		raise "No parameter required" unless opts.length == 0
# 		STDERR.puts "<<<<<<<< SKYWALK >>>>>>>>"
		nodes_along_z do |nz|
			link_full(nz)
		end
		enum = enum_nodes_cyclic
		racks_along_y do |ry|
			link_quasi_full(ry, enum)
		end
		racks_along_x do |rx|
			link_quasi_full(rx, enum)
		end
		return if !lens || lens.empty?
		raise "Just 1 length required" unless lens.length == 1
		if @cable_total < lens[0] - $innerhead
			link_random_upto(@nodes, lens[0]) do |n1, n2|
				x1, y1 = @coord_of[@rack_of[n1]]
				x2, y2 = @coord_of[@rack_of[n2]]
				x1 == x2 || y1 == y2
			end
		elsif @cable_total > lens[0]
			unlink_random_downto(lens[0]) do |n1, n2|
				@rack_of[n1] != @rack_of[n2]
			end
		end
	end
	# Fabien Special v1
	def fabien1(deg, fr, filename)
# 		STDERR.puts "<<<<<<<< FABIEN1 (#{deg}, #{filename}) >>>>>>>>"
		link_random_from(@nodes, deg, fr, filename)
	end
	# Fabien Special v2
	def fabien2(deg, fr, filename)
# 		STDERR.puts "<<<<<<<< FABIEN2 (#{deg}, #{filename}) >>>>>>>>"
		nodes_along_z do |nz|
			link_full(nz)
		end
		maxdeg = deg - (@racksize - 1)
		link_quasi_random_from(@racks, maxdeg, fr, filename)
	end
	# ラック間完全結合リンク
	def link_quasi_full(racks, enum)
		racks.combination(2) do |r1, r2|
			n1 = enum[r1].next
			n2 = enum[r2].next
# 			STDERR.puts "racks #{r1}--#{r2}\tnodes #{n1}--#{n2}"
			if !block_given? || yield(r1, r2)
				add_link(n1, n2)
			end
		end
	end
	# ラック間ランダムリンク (次数指定)
	def link_quasi_random(racks, deg, enum)
		raise "Too small degree (#{deg})" unless deg >= 0
		return if deg == 0
# 		STDERR.print "#{racks}"
		a0 = Array.new
		RAND_TRIAL.times do |t|
			a1 = Array.new
			plan = graph_of_racks
			(deg * @racksize).times do |d|
				a2 = racks.random_matching{|r1, r2|
					if plan.has_edge?(r1, r2)
						false
					elsif block_given? && !yield(r1, r2)
						false
					else
						true
					end
				}
				a1.concat(a2)
				plan.add_edges(a2)
			end
# 			STDERR.print " #{a1.length}"
			a0 = a1 if a0.length < a1.length
		end
# 		STDERR.puts " => #{a0.length} links added. #{a0} = #{a0.uniq.length}"
		degs = Hash.new(0)
		a0.each do |r1, r2|
			n1 = enum[r1].next
			n2 = enum[r2].next
			if degs[n1] < deg && degs[n2] < deg
# 				STDERR.puts "racks #{r1}--#{r2}\tnodes #{n1}--#{n2}\tdegs (#{degs[n1]})--(#{degs[n2]})"
				add_link(n1, n2)
				degs[n1] += 1
				degs[n2] += 1
			else
				STDERR.puts "racks #{r1}--#{r2}\tnodes #{n1}--#{n2}\tdegs (#{degs[n1]})--(#{degs[n2]}) ... over degree!"
			end
		end
	end
	# ラック間ランダムリンク (次数指定、ファイルから抽選)
	def link_quasi_random_from(racks, deg, fr, filename)
		raise "Too small degree (#{deg})" unless deg >= 0
		return if deg == 0
# 		STDERR.print "#{racks}"
		g = MyWeightedGraph.new
		g.load_edgelist(filename)
		rdeg = deg * @racksize
		a0 = Array.new
		RAND_TRIAL.times do |t|
			a1 = Array.new
			degs = Hash.new(0)
			g.multiple_edges.shuffle.failure(fr).each do |r1, r2|
				if degs[r1] < rdeg && degs[r2] < rdeg
					a1.push([r1, r2])
					degs[r1] += 1
					degs[r2] += 1
				end
			end
# 			STDERR.print " #{a1.length}"
			a0 = a1 if a0.length < a1.length
		end
# 		STDERR.puts " => #{a0.length} links added. #{a0} = #{a0.uniq.length}"
		degs = Hash.new(0)
		a0.each do |r1, r2|
			nodes1 = @nodes_in[r1].select{|n1| degs[n1] < deg }.shuffle
			nodes2 = @nodes_in[r2].select{|n2| degs[n2] < deg }.shuffle
			[nodes1, nodes2].self_product.each do |n1, n2|
				if !has_edge?(n1, n2)
# 					STDERR.puts "racks #{r1}--#{r2}\tnodes #{n1}--#{n2}"
					add_link(n1, n2)
					degs[n1] += 1
					degs[n2] += 1
					break
				end
			end
		end
	end
	# ラック間キューブリンク
	# 	sizes = 各次元方向のノード数
	def link_quasi_cube(racks, sizes, ring, enum)
		return @cube_index += 1 if sizes.empty?
		a = Array.new
		sizes[0].times do
			a.push(link_quasi_cube(racks, sizes[1..-1], ring, enum))
		end
		sizes[0].times do |k|
			next unless k > 0 || ring
			[[a[k - 1]].flatten, [a[k]].flatten].each_pair do |i, j|
				next unless racks[i] && racks[j] && i != j
				n1 = enum[racks[i]].next
				n2 = enum[racks[j]].next
				add_link(n1, n2)
			end
		end
		a
	end
	# ランダム (次数指定)
	def random(deg)
# 		STDERR.puts "<<<<<<<< RANDOM (#{deg}) >>>>>>>>"
		link_random(@nodes, deg)
	end
	# ラック内ランダム (次数指定)
	def random_inside(deg)
# 		STDERR.puts "<<<<<<<< RANDOM_INSIDE (#{deg}) >>>>>>>>"
		@racks.each do |r|
			link_random(@nodes_in[r], deg)
		end
	end
	# ラック外ランダム (次数指定)
	def random_outside(deg)
# 		STDERR.puts "<<<<<<<< RANDOM_OUTSIDE (#{deg}}) >>>>>>>>"
		link_random(@nodes, deg) do |n1, n2|
			@rack_of[n1] != @rack_of[n2]
		end
	end
	# ランダム (配線長指定)
	def random_upto(len)
# 		STDERR.puts "<<<<<<<< RANDOM_UPTO (#{len}) >>>>>>>>"
		link_random_upto(@nodes, len)
	end
	# ストレート
	# 	degs = [X, Y, Z] 各方向の追加次数 (1 ～ 頂点数-3)
	def straight(degs, kind)
		raise "Exactly 3 parameters required for degrees along X, Y, Z" unless degs.length == 3
# 		STDERR.puts "<<<<<<<< STRAIGHT (#{degs}, #{kind}) >>>>>>>>"
		nodes_along_z do |nz|
			link(nz, kind, degs[2])
		end
		nodes_along_y do |ny|
			link(ny, kind, degs[1])
		end
		nodes_along_x do |nx|
			link(nx, kind, degs[0]) 
		end
		return
	end
	# 線状キューブ
	# 	sizes = 各次元のノード数 [X方向, Y方向, Z方向, ...]
	def linecube(sizes)
# 		STDERR.puts "<<<<<<<< LINECUBE (#{sizes}) >>>>>>>>"
		link(@nodes, :linecube, sizes)
	end
	# 環状キューブ
	# 	sizes = 各次元のノード数 [X方向, Y方向, Z方向, ...]
	def ringcube(sizes)
# 		STDERR.puts "<<<<<<<< RINGCUBE (#{sizes}) >>>>>>>>"
		link(@nodes, :ringcube, sizes)
	end
	
	# ノード群をリンクする
	def link(nodes, kind, param = 0)
		case kind
		when :line
			link_line(nodes, false)
		when :ring
			link_line(nodes, true)
		when :fring
			link_fring(nodes)
		when :full
			link_full(nodes)
		when :linecube
			link_cube(nodes, param, false)
		when :ringcube
			link_cube(nodes, param, true)
		when :random
			link_random(nodes, param)
		end
	end
	# 線状／環状リンク
	def link_line(nodes, ring)
		return if nodes.length < 2
		nodes.each_cons(2) do |n1, n2|
			add_link(n1, n2)
		end
		add_link(nodes[0], nodes[-1]) if ring
	end
	# 折りたたみ環状リンク
	def link_fring(nodes)
		return if nodes.length < 2
		add_link(nodes[0], nodes[1]) 
		nodes.each_cons(3) do |n1, n2, n3|
			add_link(n1, n3)
		end
		add_link(nodes[-2], nodes[-1])
	end
	# 完全結合リンク
	def link_full(nodes)
		return if nodes.length < 2
		nodes.combination(2) do |n1, n2|
			add_link(n1, n2)
		end
	end
	# キューブリンク
	# 	sizes = 各次元方向のノード数
	def link_cube(nodes, sizes, ring)
		# 例: sizes = [4,3,2]
		bases = Array.new
		sizes.reverse.inject(1) do |k, l|
			bases.unshift(Array.new(l){|j| j * k })
			k * l
		end
		# 例: bases = [[0,6,12,18],[0,2,4],[0,1]]
		bases.each_index do |i|
			b = bases.dup
			h = b.delete_at(i)
			# 例: i = 1; h = [0,2,4]; b = [[0,6,12,18],[0,1]]
			b.self_product.collect{|a| a.sum }.each do |k|
				# 例: k = 0,1,6,7,12,13,18,19
				link_line(h.collect{|j| nodes[j + k] }.compact, ring)
			end
		end
	end
	# ランダムリンク (次数指定)
	def link_random(nodes, deg)
		raise "Too small degree (#{deg})" unless deg >= 0
		return if deg == 0
		deg = nodes.length - 1 if deg >= nodes.length
# 		STDERR.print "#{nodes}"
		a0 = Array.new
		RAND_TRIAL.times do |t|
			a1 = Array.new
			plan = self.dup
			deg.times do |d|
				a2 = nodes.random_matching{|n1, n2|
					if plan.has_edge?(n1, n2)
						false
					elsif block_given?
						yield(n1, n2)
					else
						true
					end
				}
				a1.concat(a2)
				plan.add_links(a2)
			end
# 			print " #{a1.length}"
			a0 = a1 if a0.length < a1.length
			break if a0.length == nodes.length / 2 * deg
		end
# 		STDERR.puts " => #{a0.length} links added. #{a0} = #{a0.uniq.length}"
		add_links(a0)
	end
	# ランダムリンク (次数指定、ファイルから抽選)
	def link_random_from(nodes, deg, fr, filename)
		raise "Too small degree (#{deg})" unless deg >= 0
		return if deg == 0
		deg = nodes.length - 1 if deg >= nodes.length
# 		STDERR.print "#{nodes}"
		g = MyGraph.new
		g.load_edgelist(filename)
		a0 = Array.new
		RAND_TRIAL.times do |t|
			a1 = Array.new
			degs = Hash.new(0)
			g.edges.shuffle.failure(fr).each do |n1, n2|
				if degs[n1] < deg && degs[n2] < deg
					a1.push([n1, n2])
					degs[n1] += 1
					degs[n2] += 1
				end
			end
# 			STDERR.print " #{a1.length}"
			a0 = a1 if a0.length < a1.length
			break if a0.length == nodes.length / 2 * deg
		end
# 		STDERR.puts " => #{a0.length} links added."  #{a0} = #{a0.uniq.length}"
		add_links(a0)
	end
	# ランダムリンク (配線長指定)
	def link_random_upto(nodes, maxcable)
		raise "Too short length (#{maxcable})" unless maxcable >= $innerhead
		total = @cable_total
		count = 0
		a1 = Array.new
		plan = self.dup
		while count < RAND_TRIAL
			a2 = nodes.random_matching{|n1, n2|
				if plan.has_edge?(n1, n2)
					false
				elsif block_given? && !yield(n1, n2)
					false
				else
					x = total + cable_between(n1, n2)
					if x > maxcable
						false
					else
						total = x
						true
					end
				end
			}
			a1.concat(a2)
			plan.add_links(a2)
			count += 1 if a2.length == 0
			break if self.edge_count + a1.length >= nodes.length * (nodes.length - 1) / 2
		end
		add_links(a1)
# 		STDERR.puts " => #{a1.length} links added. #{a1} = #{a1.uniq.length}. total = #{@cable_total.to_f / 100}m."
	end
	# ランダムリンクを削除 (配線長指定)
	def unlink_random_downto(maxcable)
		raise "Too short length (#{maxcable})" unless maxcable >= $innerhead
		a1 = Array.new
		pool = self.edges.shuffle
		until pool.empty?
			n1, n2 = pool.shift
			if !block_given? || yield(n1, n2)
				a1.push([n1, n2])
				delete_link(n1, n2)
				break if @cable_total < maxcable
			end
		end
# 		STDERR.puts " => #{a1.length} links deleted. #{a1} = #{a1.uniq.length}. cable = #{@cable_total.to_f / 100}m."
	end
end

main if $0 == __FILE__
