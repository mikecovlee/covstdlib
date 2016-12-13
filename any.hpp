namespace cov {
	class any final {
		class baseHolder {
		public:
			baseHolder() = default;
			virtual ~ baseHolder() = default;
			virtual const std::type_info & type() const = 0;
			virtual baseHolder *duplicate() = 0;
			virtual bool compare(const baseHolder *) const = 0;
			virtual std::string to_string() const = 0;
		};
		template < typename T > class holder:public baseHolder {
		protected:
			T mDat;
		public:
			holder() = default;
			holder(const T& dat):mDat(dat) {}
			virtual ~ holder() = default;
			virtual const std::type_info & type() const override
			{
				return typeid(T);
			}
			virtual baseHolder *duplicate() override
			{
				return new holder(mDat);
			}
			virtual bool compare(const baseHolder * obj)const override
			{
				if (obj->type() == this->type()) {
					const holder < T > *ptr = dynamic_cast < const holder < T > *>(obj);
					return ptr!=nullptr?mDat == ptr->data():false;
				}
				return false;
			}
			virtual std::string to_string() const override
			{
				return std::move(std::to_string(mDat));
			}
			T & data()
			{
				return mDat;
			}
			const T & data() const
			{
				return mDat;
			}
			void data(const T & dat)
			{
				mDat = dat;
			}
		};
		baseHolder * mDat=nullptr;
	public:
		static any infer_value(const std::string&);
		void swap(any& obj) noexcept {
			baseHolder* tmp=this->mDat;
			this->mDat=obj.mDat;
			obj.mDat=tmp;
		}
		void swap(any&& obj)
		{
			baseHolder* tmp=this->mDat;
			this->mDat=obj.mDat;
			obj.mDat=tmp;
		}
		bool usable() const noexcept
		{
			return mDat != nullptr;
		}
		any()=default;
		template < typename T > any(const T & dat):mDat(new holder < T > (dat)) {}
		any(const any & v):mDat(v.usable()?v.mDat->duplicate():nullptr) {}
		any(any&& v) noexcept {
			swap(std::forward<any>(v));
		}
		~any()
		{
			delete mDat;
		}
		const std::type_info & type() const
		{
			return this->mDat != nullptr?this->mDat->type():typeid(void);
		}
		std::string to_string() const
		{
			if(this->mDat == nullptr)
				throw cov::error("E0005");
			return std::move(this->mDat->to_string());
		}
		any & operator=(const any & var)
		{
			if(&var!=this) {
				delete mDat;
				mDat = var.usable()?var.mDat->duplicate():nullptr;
			}
			return *this;
		}
		any & operator=(any&& var) noexcept {
			if(&var!=this)
				swap(std::forward<any>(var));
			return *this;
		}
		bool operator==(const any & var) const
		{
			return usable()?this->mDat->compare(var.mDat):!var.usable();
		}
		bool operator!=(const any & var)const
		{
			return usable()?!this->mDat->compare(var.mDat):var.usable();
		}
		template < typename T > T & val()
		{
			if(typeid(T) != this->type())
				throw cov::error("E0006");
			if(this->mDat == nullptr)
				throw cov::error("E0005");
			return dynamic_cast < holder < T > *>(this->mDat)->data();
		}
		template < typename T > const T & val() const
		{
			if(typeid(T) != this->type())
				throw cov::error("E0006");
			if(this->mDat == nullptr)
				throw cov::error("E0005");
			return dynamic_cast < const holder < T > *>(this->mDat)->data();
		}
		template < typename T > operator T&()
		{
			return this->val<T>();
		}
		template < typename T > operator const T&() const
		{
			return this->val<T>();
		}
		template < typename T > void assign(const T & dat)
		{
			delete mDat;
			mDat = new holder < T > (dat);
		}
		template < typename T > any & operator=(const T & dat)
		{
			assign(dat);
			return *this;
		}
	};
	template<int N> class any::holder<char[N]>:public any::holder<std::string> {
	public:
		using holder<std::string>::holder;
	};
	template<> class any::holder<std::type_info>:public any::holder<std::type_index> {
	public:
		using holder<std::type_index>::holder;
	};
}